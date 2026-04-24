#include <not_implemented.h>
#include "../include/allocator_sorted_list.h"

allocator_sorted_list::~allocator_sorted_list()
{
    if (!_trusted_memory) return;

    char* base = static_cast<char*>(_trusted_memory);
    auto* parent_ptr = *reinterpret_cast<std::pmr::memory_resource**>(base);
    base += sizeof(std::pmr::memory_resource *);
    base += sizeof(allocator_with_fit_mode::fit_mode);
    auto* space_size_ptr = reinterpret_cast<size_t*>(base);
    size_t total_size = *space_size_ptr;
    base += sizeof(size_t);
    auto* mutex_ptr = reinterpret_cast<std::mutex*>(base);
    mutex_ptr->~mutex();

    if (parent_ptr)
        parent_ptr->deallocate(_trusted_memory, total_size);
    else
        ::operator delete(_trusted_memory);
    _trusted_memory = nullptr;
}

allocator_sorted_list::allocator_sorted_list(
    allocator_sorted_list &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_sorted_list &allocator_sorted_list::operator=(
    allocator_sorted_list &&other) noexcept
{
    if (this != &other)
    {
        this->~allocator_sorted_list();

        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    return *this;
}

allocator_sorted_list::allocator_sorted_list(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size < allocator_metadata_size + block_metadata_size)
        throw std::bad_alloc();

    if (parent_allocator)
        _trusted_memory = parent_allocator->allocate(space_size);
    else
        _trusted_memory = ::operator new(space_size);
    char* base = static_cast<char*>(_trusted_memory);

    auto* parent_ptr = reinterpret_cast<std::pmr::memory_resource**>(base);
    *parent_ptr = parent_allocator;
    base += sizeof(std::pmr::memory_resource*);

    auto* fit_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(base);
    *fit_ptr = allocate_fit_mode;
    base += sizeof(allocator_with_fit_mode::fit_mode);

    auto* space_size_ptr = reinterpret_cast<size_t*>(base);
    *space_size_ptr = space_size;
    base += sizeof(size_t);

    auto* mutex_ptr = reinterpret_cast<std::mutex*>(base);
    new (mutex_ptr) std::mutex();
    base += sizeof(std::mutex);

    auto* free_head_ptr = reinterpret_cast<void**>(base);
    base += sizeof(void*);

    char* first_block = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    *free_head_ptr = first_block;

    *reinterpret_cast<void**>(first_block) = nullptr;
    size_t block_size = space_size - allocator_metadata_size - block_metadata_size;
    *reinterpret_cast<size_t*>(first_block + sizeof(void*)) = block_size;
}
   

[[nodiscard]] void *allocator_sorted_list::do_allocate_sm(
    size_t size)
{
    if (size == 0) throw std::bad_alloc();

    char* base = static_cast<char*>(_trusted_memory);
    size_t space_size = *reinterpret_cast<size_t*>(
        base
        + sizeof(std::pmr::memory_resource*)
        + sizeof(allocator_with_fit_mode::fit_mode)
    );

    constexpr size_t min_block_overhead = sizeof(void*) + sizeof(size_t);
    if (size > space_size - min_block_overhead - allocator_metadata_size - block_metadata_size)
        throw std::bad_alloc();

    base += sizeof(std::pmr::memory_resource *);
    auto* fit_mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(base);
    allocator_with_fit_mode::fit_mode mode = *fit_mode_ptr;
    base += sizeof(allocator_with_fit_mode::fit_mode);
    size_t total_size = *reinterpret_cast<size_t*>(base);
    base += sizeof(size_t);
    auto* mutex_ptr = reinterpret_cast<std::mutex*>(base);
    std::lock_guard<std::mutex> lock(*mutex_ptr);
    base += sizeof(std::mutex);
    auto* free_head_ptr = reinterpret_cast<void**>(base);

    void* best = nullptr;
    void* best_prev = nullptr;

    void* prev = nullptr;
    void* current = *free_head_ptr;
    size_t best_size = 0;
    size_t worst_size = 0;
    size_t found = 0;

    while (current)
    {
        auto* header = reinterpret_cast<char*>(current);
        size_t block_size = *reinterpret_cast<size_t*>(header + sizeof(void*));

        if (block_size >= size)
        {
            switch (mode)
            {
                case allocator_with_fit_mode::fit_mode::first_fit:
                {
                    best = current;
                    best_prev = prev;
                    current = nullptr;
                    found = 1;
                    break;
                }
                case allocator_with_fit_mode::fit_mode::the_best_fit:
                {
                    if (!best || block_size < best_size)
                    {
                        best = current;
                        best_prev = prev;
                        best_size = block_size;
                    }
                    break;
                }
                case allocator_with_fit_mode::fit_mode::the_worst_fit:
                {
                    if (block_size > worst_size)
                    {
                        best = current;
                        best_prev = prev;
                        worst_size = block_size;
                    }
                    break;
                }
            }
            if (found) break;
        }
        prev = current;
        auto* next_ptr = reinterpret_cast<void**>(current);
        current = *next_ptr;
    }
    if (!best) throw std::bad_alloc();

    void* next = *reinterpret_cast<void**>(best);
    if (best_prev)
        *reinterpret_cast<void**>(best_prev) = next;
    else
        *free_head_ptr = next;
    auto* block = reinterpret_cast<char*>(best);
    size_t block_size = *reinterpret_cast<size_t*>(block + sizeof(void*));


    char* begin = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    char* end   = static_cast<char*>(_trusted_memory) + total_size;
    constexpr size_t min_block_size = sizeof(void*) + sizeof(size_t);

    size_t header_size = sizeof(void*) + sizeof(size_t);
    size_t full_block_size = block_size + header_size;

    if (full_block_size >= size + header_size + min_block_size)
    {
        size_t remaining = full_block_size - size - header_size;
        *reinterpret_cast<size_t*>(block + sizeof(void*)) = size;
        if (remaining < min_block_size)
        {
            return block + sizeof(void*) + sizeof(size_t);
        }
        auto* new_block = block + header_size + size;
        auto* new_next = reinterpret_cast<void**>(new_block);
        auto* new_size = reinterpret_cast<size_t*>(new_block + sizeof(void*));

        

        *new_size = remaining - header_size;
        void* curr = *free_head_ptr;
        void* prev = nullptr;

        while (curr && curr >= begin && curr < end &&
            static_cast<char*>(curr) < static_cast<char*>(new_block))
        {
            prev = curr;
            curr = *reinterpret_cast<void**>(curr);
        }

        char* tmp = static_cast<char*>(*free_head_ptr);

        int guard = 0;
        while (tmp && tmp >= begin && tmp < end)
        {
            if (++guard > 10000)
                throw std::logic_error("free list cycle detected");

            if (tmp == new_block)
                throw std::logic_error("duplicate free block");

            tmp = *reinterpret_cast<char**>(tmp);
        }
        *reinterpret_cast<void**>(new_block) = curr;

        if (prev)
            *reinterpret_cast<void**>(prev) = new_block;
        else
            *free_head_ptr = new_block;
    }

    return block + sizeof(void*) + sizeof(size_t);
}

allocator_sorted_list::allocator_sorted_list(const allocator_sorted_list &other)
{
    throw std::logic_error("copy assignment not supported");
}

allocator_sorted_list &allocator_sorted_list::operator=(const allocator_sorted_list &other)
{
    // copy not supported, but we can move
    throw std::logic_error("copy assignment not supported");
}

bool allocator_sorted_list::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    auto* o = dynamic_cast<const allocator_sorted_list*>(&other);
    return o && o->_trusted_memory == _trusted_memory;
}

void allocator_sorted_list::do_deallocate_sm(void *at)
{
    if (!at) return;

    char* base = static_cast<char*>(_trusted_memory);
    base += sizeof(std::pmr::memory_resource *);
    base += sizeof(allocator_with_fit_mode::fit_mode);

    size_t total_size = *reinterpret_cast<size_t*>(base);
    base += sizeof(size_t);

    auto* mutex_ptr = reinterpret_cast<std::mutex*>(base);
    base += sizeof(std::mutex);

    auto* free_head_ptr = reinterpret_cast<void**>(base);

    std::lock_guard<std::mutex> lock(*mutex_ptr);

    char* begin = static_cast<char*>(_trusted_memory) + allocator_metadata_size;
    char* end   = static_cast<char*>(_trusted_memory) + total_size;

    char* block = static_cast<char*>(at) - sizeof(void*) - sizeof(size_t);

    if (block < begin || block >= end)
        return;

    size_t size = *reinterpret_cast<size_t*>(block + sizeof(void*));

    void* current = *free_head_ptr;
    void* prev = nullptr;

    while (current && current < block)
    {
        prev = current;
        current = *reinterpret_cast<void**>(current);
    }
    if (prev)
    {
        char* prev_ptr = static_cast<char*>(prev);
        size_t prev_size =
            *reinterpret_cast<size_t*>(prev_ptr + sizeof(void*));

        char* prev_end =
            prev_ptr + sizeof(void*) + sizeof(size_t) + prev_size;

        if (prev_end == block)
        {
            void* prev_next = *reinterpret_cast<void**>(prev);

            if (prev == *free_head_ptr)
                *free_head_ptr = prev_next;
            else
            {
                void* tmp = *free_head_ptr;
                while (tmp && *reinterpret_cast<void**>(tmp) != prev)
                    tmp = *reinterpret_cast<void**>(tmp);

                if (tmp)
                    *reinterpret_cast<void**>(tmp) = prev_next;
            }

            size_t new_size =
                prev_size + size + sizeof(void*) + sizeof(size_t);

            *reinterpret_cast<size_t*>(prev_ptr + sizeof(void*)) = new_size;

            block = prev_ptr;
            size = new_size;
            prev = nullptr;
        }
        current = *reinterpret_cast<void**>(block);
    }

    if (current)
    {
        char* curr_ptr = static_cast<char*>(current);

        char* block_end =
            block + sizeof(void*) + sizeof(size_t) + size;

        if (block_end == curr_ptr)
        {
            size_t curr_size =
                *reinterpret_cast<size_t*>(curr_ptr + sizeof(void*));

            size += sizeof(void*) + sizeof(size_t) + curr_size;

            *reinterpret_cast<size_t*>(block + sizeof(void*)) = size;

            current = *reinterpret_cast<void**>(curr_ptr);
        }
    }

    *reinterpret_cast<void**>(block) = current;

    if (prev)
        *reinterpret_cast<void**>(prev) = block;
    else
        *free_head_ptr = block;
}

inline void allocator_sorted_list::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    char* base = static_cast<char*>(_trusted_memory);
    base += sizeof(std::pmr::memory_resource *);

    auto* fit_mode_ptr = reinterpret_cast<allocator_with_fit_mode::fit_mode*>(base);

    *fit_mode_ptr = mode;
}

std::vector<allocator_test_utils::block_info> allocator_sorted_list::get_blocks_info() const noexcept
{
    std::vector<allocator_test_utils::block_info> result;

    if (!_trusted_memory)
        return result;

    char* begin = static_cast<char*>(_trusted_memory) + allocator_metadata_size;

    char* base = static_cast<char*>(_trusted_memory);
    base += sizeof(std::pmr::memory_resource *) + sizeof(allocator_with_fit_mode::fit_mode);
    size_t total_size = *reinterpret_cast<size_t*>(base);
    base += sizeof(size_t);

    auto* mutex_ptr = reinterpret_cast<std::mutex*>(base);
    char* end = static_cast<char*>(_trusted_memory) + total_size;

    std::lock_guard<std::mutex> lock(*mutex_ptr);
    std::vector<void*> free_blocks;

    base += sizeof(std::mutex);
    auto* free_head_ptr = reinterpret_cast<void**>(base);

    void* current = *free_head_ptr;
    while (current)
    {
        free_blocks.push_back(current);
        auto* next_ptr = reinterpret_cast<void**>(current);
        current = *next_ptr;
    }

    char* ptr = begin;
    while (ptr < end)
    {
        size_t size = *reinterpret_cast<size_t*>(ptr + sizeof(void*));
        bool is_free = std::find(free_blocks.begin(), free_blocks.end(), ptr) != free_blocks.end();
        result.push_back({size, !is_free});
        ptr += sizeof(void*) + sizeof(size_t) + size;
    }
    return result;
}


std::vector<allocator_test_utils::block_info>
allocator_sorted_list::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> result;

    if (!_trusted_memory)
        return result;

    char* base = static_cast<char*>(_trusted_memory);

    size_t total_size =
        *reinterpret_cast<size_t*>(
            base
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
        );

    char* begin = base + allocator_metadata_size;
    char* end = base + total_size;

    std::vector<void*> free_blocks;

    auto* free_head =
        reinterpret_cast<void**>(
            base
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t)
            + sizeof(std::mutex)
        );

    void* curr = *free_head;

    while (curr)
    {
        free_blocks.push_back(curr);
        curr = *reinterpret_cast<void**>(curr);
    }

    char* ptr = begin;

    while (ptr < end)
    {
        if (ptr < begin || ptr + sizeof(void*) + sizeof(size_t) > end) break;
        size_t size =
            *reinterpret_cast<size_t*>(ptr + sizeof(void*));

        bool is_free =
            std::find(free_blocks.begin(), free_blocks.end(), ptr)
            != free_blocks.end();

        result.push_back({
            size,
            is_free
        });

        ptr += sizeof(void*) + sizeof(size_t) + size;
    }

    return result;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_begin() const noexcept
{
    auto* base = static_cast<char*>(_trusted_memory);

    auto* free_head =
        reinterpret_cast<void**>(
            base
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t)
            + sizeof(std::mutex)
        );

    return sorted_free_iterator(*free_head);
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::free_end() const noexcept
{
    return sorted_free_iterator(nullptr);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::begin() const noexcept
{
    if (!_trusted_memory)
        return sorted_iterator(nullptr);

    char* base = static_cast<char*>(_trusted_memory);

    return sorted_iterator(base + allocator_metadata_size);
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::end() const noexcept
{
    if (!_trusted_memory)
        return sorted_iterator(nullptr);

    char* base = static_cast<char*>(_trusted_memory);

    size_t total_size =
        *reinterpret_cast<size_t*>(
            base
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
        );

    return sorted_iterator(base + total_size);
}


bool allocator_sorted_list::sorted_free_iterator::operator==(
        const allocator_sorted_list::sorted_free_iterator & other) const noexcept
{
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_free_iterator::operator!=(
        const allocator_sorted_list::sorted_free_iterator &other) const noexcept
{
    return _free_ptr != other._free_ptr;
}

allocator_sorted_list::sorted_free_iterator &allocator_sorted_list::sorted_free_iterator::operator++() & noexcept
{
    if (_free_ptr)
    {
        _free_ptr = *reinterpret_cast<void**>(_free_ptr);
    }
    return *this;
}

allocator_sorted_list::sorted_free_iterator allocator_sorted_list::sorted_free_iterator::operator++(int n)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

size_t allocator_sorted_list::sorted_free_iterator::size() const noexcept
{
    if (!_free_ptr) return 0;
    char* block = static_cast<char*>(_free_ptr);
    return *reinterpret_cast<size_t*>(block + sizeof(void*));
}

void *allocator_sorted_list::sorted_free_iterator::operator*() const noexcept
{
    return _free_ptr;
}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator() : _free_ptr(nullptr)
{}

allocator_sorted_list::sorted_free_iterator::sorted_free_iterator(void *trusted) : _free_ptr(trusted)
{}

bool allocator_sorted_list::sorted_iterator::operator==(const allocator_sorted_list::sorted_iterator & other) const noexcept
{
    return _free_ptr == other._free_ptr;
}

bool allocator_sorted_list::sorted_iterator::operator!=(const allocator_sorted_list::sorted_iterator &other) const noexcept
{
    return _free_ptr != other._free_ptr;
}

allocator_sorted_list::sorted_iterator &allocator_sorted_list::sorted_iterator::operator++() & noexcept
{
    if (_free_ptr)
    {
        size_t size =
            *reinterpret_cast<size_t*>(
                static_cast<char*>(_free_ptr) + sizeof(void*)
            );

        _free_ptr = static_cast<char*>(_free_ptr)
            + sizeof(void*) + sizeof(size_t) + size;
        if (size == 0 || size > 1'000'000)
        {
            _free_ptr = nullptr;
            return *this;
        }
    }
    return *this;
}

allocator_sorted_list::sorted_iterator allocator_sorted_list::sorted_iterator::operator++(int n)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

size_t allocator_sorted_list::sorted_iterator::size() const noexcept
{
    if (!_free_ptr) return 0;

    return *reinterpret_cast<size_t*>(
        static_cast<char*>(_free_ptr) + sizeof(void*)
    );
}

void *allocator_sorted_list::sorted_iterator::operator*() const noexcept
{
    return _free_ptr;
}

allocator_sorted_list::sorted_iterator::sorted_iterator() : _free_ptr(nullptr)
{}

allocator_sorted_list::sorted_iterator::sorted_iterator(void *trusted) : _free_ptr(trusted)
{}

bool allocator_sorted_list::sorted_iterator::occupied() const noexcept
{
    if (!_free_ptr) return false;
    return true;
}