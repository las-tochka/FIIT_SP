#include <not_implemented.h>
#include "../include/allocator_boundary_tags.h"

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory)
    {
        auto parent = std::pmr::get_default_resource();
        parent->deallocate(_trusted_memory, 0);
        _trusted_memory = nullptr;
    }
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags &&other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_boundary_tags &allocator_boundary_tags::operator=(
    allocator_boundary_tags &&other) noexcept
{
    if (this != &other)
    {
        _trusted_memory = other._trusted_memory;
        other._trusted_memory = nullptr;
    }
    return *this;
}

/** If parent_allocator* == nullptr you should use std::pmr::get_default_resource()
 */
allocator_boundary_tags::allocator_boundary_tags(
        size_t space_size,
        std::pmr::memory_resource *parent_allocator,
        allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (!parent_allocator)
        parent_allocator = std::pmr::get_default_resource();

    _trusted_memory = parent_allocator->allocate(space_size);

    *((allocator_with_fit_mode::fit_mode*)_trusted_memory) = allocate_fit_mode;

    std::mutex* m = (std::mutex*)((char*)_trusted_memory + sizeof(allocator_with_fit_mode::fit_mode));
    new (m) std::mutex();

    *((size_t*)((char*)_trusted_memory +
        sizeof(allocator_with_fit_mode::fit_mode) +
        sizeof(std::mutex))) = space_size;

    void* first = (char*)_trusted_memory + allocator_metadata_size;

    *((size_t*)first) = space_size - allocator_metadata_size;
    *((bool*)((char*)first + sizeof(size_t))) = true;
}

[[nodiscard]] void *allocator_boundary_tags::do_allocate_sm(
    size_t size)
{
     std::mutex* m = (std::mutex*)((char*)_trusted_memory + sizeof(allocator_with_fit_mode::fit_mode));
    std::lock_guard<std::mutex> lock(*m);

    void* best = nullptr;

    auto mode = *((allocator_with_fit_mode::fit_mode*)_trusted_memory);

    size_t mem_size = *((size_t*)((char*)_trusted_memory +
        sizeof(allocator_with_fit_mode::fit_mode) +
        sizeof(std::mutex)));

    char* cur = (char*)_trusted_memory + allocator_metadata_size;

    while (cur < (char*)_trusted_memory + mem_size)
    {
        size_t block_size = *((size_t*)cur);
        bool free = *((bool*)(cur + sizeof(size_t)));

        if (free && block_size >= size)
        {
            if (mode == allocator_with_fit_mode::fit_mode::first_fit)
            {
                best = cur;
                break;
            }

            if (mode == allocator_with_fit_mode::fit_mode::the_best_fit)
            {
                if (!best || block_size < *((size_t*)best))
                    best = cur;
            }

            if (mode == allocator_with_fit_mode::fit_mode::the_worst_fit)
            {
                if (!best || block_size > *((size_t*)best))
                    best = cur;
            }
        }

        cur += sizeof(size_t) + sizeof(bool) + block_size;
    }

    if (!best)
        return nullptr;
    

    size_t old_size = *((size_t*)best);

    if (old_size > size + sizeof(size_t) + sizeof(bool) + 8)
    {
        char* new_block = (char*)best + sizeof(size_t) + sizeof(bool) + size;

        size_t remain = old_size - size - sizeof(size_t) - sizeof(bool);

        *((size_t*)new_block) = remain;
        *((bool*)(new_block + sizeof(size_t))) = true;

        *((size_t*)best) = size;
    }

    *((bool*)((char*)best + sizeof(size_t))) = false;

    return (char*)best + sizeof(size_t) + sizeof(bool);
}

void allocator_boundary_tags::do_deallocate_sm(
    void *at)
{
    if (!at) return;

    std::mutex* m = (std::mutex*)((char*)_trusted_memory + sizeof(allocator_with_fit_mode::fit_mode));
    std::lock_guard<std::mutex> lock(*m);

    char* block = (char*)at - (sizeof(size_t) + sizeof(bool));

    *((bool*)(block + sizeof(size_t))) = true;

    // simple merge forward
    size_t mem_size = *((size_t*)((char*)_trusted_memory +
        sizeof(allocator_with_fit_mode::fit_mode) +
        sizeof(std::mutex)));

    size_t cur_size = *((size_t*)block);

    char* next = block + sizeof(size_t) + sizeof(bool) + cur_size;

    while (next < (char*)_trusted_memory + mem_size)
    {
        size_t next_size = *((size_t*)next);
        bool next_free = *((bool*)(next + sizeof(size_t)));

        if (!next_free)
            break;

        cur_size += sizeof(size_t) + sizeof(bool) + next_size;

        *((size_t*)block) = cur_size;

        next = block + sizeof(size_t) + sizeof(bool) + cur_size;
    }
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *((allocator_with_fit_mode::fit_mode*)_trusted_memory) = mode;
}


std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info() const
{
    std::mutex* m = (std::mutex*)((char*)_trusted_memory + sizeof(allocator_with_fit_mode::fit_mode));
    std::lock_guard<std::mutex> lock(*m);

    return get_blocks_info_inner();
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::begin() const noexcept
{return boundary_iterator((char*)_trusted_memory + allocator_metadata_size);
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::end() const noexcept
{
        size_t mem_size = *((size_t*)((char*)_trusted_memory +
        sizeof(allocator_with_fit_mode::fit_mode) +
        sizeof(std::mutex)));

    return boundary_iterator((char*)_trusted_memory + mem_size);
}

std::vector<allocator_test_utils::block_info> allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<block_info> res;

    char* cur = (char*)_trusted_memory + allocator_metadata_size;

    size_t mem_size = *((size_t*)((char*)_trusted_memory +
        sizeof(allocator_with_fit_mode::fit_mode) +
        sizeof(std::mutex)));

    while (cur < (char*)_trusted_memory + mem_size)
    {
        size_t size = *((size_t*)cur);
        bool free = *((bool*)(cur + sizeof(size_t)));

        res.push_back({size, !free});

        cur += sizeof(size_t) + sizeof(bool) + size;
    }

    return res;
}

allocator_boundary_tags::allocator_boundary_tags(const allocator_boundary_tags &other)
{
    _trusted_memory = other._trusted_memory;   
}

allocator_boundary_tags &allocator_boundary_tags::operator=(const allocator_boundary_tags &other)
{
    if (this != &other)
    {
        _trusted_memory = other._trusted_memory;
    }
    return *this;
}

bool allocator_boundary_tags::do_is_equal(const std::pmr::memory_resource &other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
        const allocator_boundary_tags::boundary_iterator &other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
        const allocator_boundary_tags::boundary_iterator & other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (!_occupied_ptr) return *this;

    size_t size = *((size_t*)_occupied_ptr);
    _occupied_ptr = (char*)_occupied_ptr + sizeof(size_t) + sizeof(bool) + size;

    return *this;
}

allocator_boundary_tags::boundary_iterator &allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    return *this;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator++(int n)
{
    boundary_iterator tmp = *this;
    ++(*this);
    return tmp;
}

allocator_boundary_tags::boundary_iterator allocator_boundary_tags::boundary_iterator::operator--(int n)
{
    boundary_iterator tmp = *this;
    --(*this);
    return tmp;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
        if (!_occupied_ptr) return 0;
    return *((size_t*)((char*)_occupied_ptr));
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    if (!_occupied_ptr) return false;
    return !(*((bool*)((char*)_occupied_ptr + sizeof(size_t))));
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return (char*)_occupied_ptr;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
{
    _occupied_ptr = nullptr;
    _trusted_memory = nullptr;
    _occupied = false;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(void *trusted)
{
    _occupied_ptr = trusted;
    _trusted_memory = trusted;
    _occupied = true;
}

void *allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}