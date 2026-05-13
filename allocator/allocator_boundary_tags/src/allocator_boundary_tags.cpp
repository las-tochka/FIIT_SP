#include "../include/allocator_boundary_tags.h"

#include <new>
#include <cstring>
#include <stdexcept>

namespace
{
    constexpr size_t OCCUPIED_MASK = 1ULL;

    inline size_t align_up(size_t x)
    {
        constexpr size_t ALIGN = alignof(std::max_align_t);
        return (x + ALIGN - 1) & ~(ALIGN - 1);
    }

    struct block_header
    {
        size_t size_and_flags;
        void* prev;
        void* next;
        void* owner;
    };

    inline size_t block_size(block_header* h)
    {
        return h->size_and_flags & ~OCCUPIED_MASK;
    }

    inline bool occupied(block_header* h)
    {
        return h->size_and_flags & OCCUPIED_MASK;
    }

    inline void set_occupied(block_header* h, bool value)
    {
        if (value)
            h->size_and_flags |= OCCUPIED_MASK;
        else
            h->size_and_flags &= ~OCCUPIED_MASK;
    }

    inline void set_size(block_header* h, size_t sz)
    {
        bool occ = occupied(h);
        h->size_and_flags = sz;
        set_occupied(h, occ);
    }
}

allocator_boundary_tags::~allocator_boundary_tags()
{
    if (_trusted_memory == nullptr)
        return;

    auto parent =
        *reinterpret_cast<std::pmr::memory_resource**>(_trusted_memory);

    auto total_size =
        *reinterpret_cast<size_t*>(
            reinterpret_cast<char*>(_trusted_memory)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode));

    parent->deallocate(_trusted_memory,
                       total_size + allocator_metadata_size);

    _trusted_memory = nullptr;
}

allocator_boundary_tags::allocator_boundary_tags(
    allocator_boundary_tags&& other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_boundary_tags&
allocator_boundary_tags::operator=(
    allocator_boundary_tags&& other) noexcept
{
    if (this == &other)
        return *this;

    this->~allocator_boundary_tags();

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}

allocator_boundary_tags::allocator_boundary_tags(
    size_t space_size,
    std::pmr::memory_resource* parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (parent_allocator == nullptr)
        parent_allocator = std::pmr::get_default_resource();

    size_t total_size =
        allocator_metadata_size + space_size;

    _trusted_memory =
        parent_allocator->allocate(total_size);

    char* ptr = reinterpret_cast<char*>(_trusted_memory);

    *reinterpret_cast<std::pmr::memory_resource**>(ptr)
        = parent_allocator;

    ptr += sizeof(std::pmr::memory_resource*);

    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(ptr)
        = allocate_fit_mode;

    ptr += sizeof(allocator_with_fit_mode::fit_mode);

    *reinterpret_cast<size_t*>(ptr)
        = space_size;

    ptr += sizeof(size_t);

    new(ptr) std::mutex;

    ptr += sizeof(std::mutex);

    block_header* first =
        reinterpret_cast<block_header*>(
            reinterpret_cast<char*>(_trusted_memory)
            + allocator_metadata_size);

    first->size_and_flags = space_size;
    set_occupied(first, false);

    first->prev = nullptr;
    first->next = nullptr;
    first->owner = this;
}

[[nodiscard]]
void* allocator_boundary_tags::do_allocate_sm(size_t size)
{
    size_t required =
        size + occupied_block_metadata_size;

    auto mutex_ptr =
        reinterpret_cast<std::mutex*>(
            reinterpret_cast<char*>(_trusted_memory)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t));

    std::lock_guard guard(*mutex_ptr);

    auto mode =
        *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(
            reinterpret_cast<char*>(_trusted_memory)
            + sizeof(std::pmr::memory_resource*));

    block_header* best = nullptr;

    for (auto it = begin(); it != end(); ++it)
    {
        auto* current =
            reinterpret_cast<block_header*>(it.get_ptr());

        if (occupied(current))
            continue;

        if (block_size(current) < required)
            continue;

        switch (mode)
        {
            case fit_mode::first_fit:
                best = current;
                goto FOUND;

            case fit_mode::the_best_fit:
            {
                if (best == nullptr ||
                    block_size(current) < block_size(best))
                {
                    best = current;
                }
                break;
            }

            case fit_mode::the_worst_fit:
            {
                if (best == nullptr ||
                    block_size(current) > block_size(best))
                {
                    best = current;
                }
                break;
            }
        }
    }

FOUND:

    if (best == nullptr)
        throw std::bad_alloc();

    size_t current_size = block_size(best);

    size_t remainder =
        current_size - required;

    if (remainder >= occupied_block_metadata_size)
    {
        auto* next =
            reinterpret_cast<block_header*>(
                reinterpret_cast<char*>(best)
                + required);

        next->size_and_flags = remainder;
        set_occupied(next, false);

        next->prev = best;
        next->next = best->next;
        next->owner = this;

        if (best->next)
        {
            reinterpret_cast<block_header*>(best->next)
                ->prev = next;
        }

        best->next = next;

        set_size(best, required);
    }

    set_occupied(best, true);

    return reinterpret_cast<char*>(best)
           + occupied_block_metadata_size;
}

void allocator_boundary_tags::do_deallocate_sm(void* at)
{
    if (at == nullptr)
        return;

    auto mutex_ptr =
        reinterpret_cast<std::mutex*>(
            reinterpret_cast<char*>(_trusted_memory)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t));

    std::lock_guard guard(*mutex_ptr);

    auto* current =
        reinterpret_cast<block_header*>(
            reinterpret_cast<char*>(at)
            - occupied_block_metadata_size);

    if (current->owner != this)
        throw std::logic_error("foreign block");

    set_occupied(current, false);

    auto* next =
        reinterpret_cast<block_header*>(current->next);

    if (next && !occupied(next))
    {
        set_size(current,
                 block_size(current)
                 + block_size(next));

        current->next = next->next;

        if (next->next)
        {
            reinterpret_cast<block_header*>(next->next)
                ->prev = current;
        }
    }

    auto* prev =
        reinterpret_cast<block_header*>(current->prev);

    if (prev && !occupied(prev))
    {
        set_size(prev,
                 block_size(prev)
                 + block_size(current));

        prev->next = current->next;

        if (current->next)
        {
            reinterpret_cast<block_header*>(current->next)
                ->prev = prev;
        }
    }
}

inline void allocator_boundary_tags::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    *reinterpret_cast<allocator_with_fit_mode::fit_mode*>(
        reinterpret_cast<char*>(_trusted_memory)
        + sizeof(std::pmr::memory_resource*)) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info() const
{
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<block_info> result;

    for (auto it = begin(); it != end(); ++it)
    {
        auto* h =
            reinterpret_cast<block_header*>(it.get_ptr());

        result.push_back({
            block_size(h),
            occupied(h)
        });
    }

    return result;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::begin() const noexcept
{
    boundary_iterator it;

    it._trusted_memory = _trusted_memory;

    it._occupied_ptr =
        reinterpret_cast<char*>(_trusted_memory)
        + allocator_metadata_size;

    return it;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::end() const noexcept
{
    boundary_iterator it;

    it._trusted_memory = _trusted_memory;
    it._occupied_ptr = nullptr;

    return it;
}

allocator_boundary_tags::allocator_boundary_tags(
    const allocator_boundary_tags& other)
{
    throw std::logic_error("copy forbidden");
}

allocator_boundary_tags&
allocator_boundary_tags::operator=(
    const allocator_boundary_tags& other)
{
    throw std::logic_error("copy forbidden");
}

bool allocator_boundary_tags::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
    const boundary_iterator& other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const boundary_iterator& other) const noexcept
{
    return !(*this == other);
}

allocator_boundary_tags::boundary_iterator&
allocator_boundary_tags::boundary_iterator::operator++() & noexcept
{
    if (_occupied_ptr == nullptr)
        return *this;

    auto* h =
        reinterpret_cast<block_header*>(_occupied_ptr);

    _occupied_ptr = h->next;

    return *this;
}

allocator_boundary_tags::boundary_iterator&
allocator_boundary_tags::boundary_iterator::operator--() & noexcept
{
    if (_occupied_ptr == nullptr)
        return *this;

    auto* h =
        reinterpret_cast<block_header*>(_occupied_ptr);

    _occupied_ptr = h->prev;

    return *this;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator++(int)
{
    auto copy = *this;
    ++(*this);
    return copy;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator--(int)
{
    auto copy = *this;
    --(*this);
    return copy;
}

size_t allocator_boundary_tags::boundary_iterator::size() const noexcept
{
    auto* h =
        reinterpret_cast<block_header*>(_occupied_ptr);

    return block_size(h);
}

bool allocator_boundary_tags::boundary_iterator::occupied() const noexcept
{
    auto* h =
        reinterpret_cast<block_header*>(_occupied_ptr);

    return ::occupied(h);
}

void* allocator_boundary_tags::boundary_iterator::operator*() const noexcept
{
    return reinterpret_cast<char*>(_occupied_ptr)
           + occupied_block_metadata_size;
}

allocator_boundary_tags::boundary_iterator::boundary_iterator()
    : _occupied_ptr(nullptr),
      _occupied(false),
      _trusted_memory(nullptr)
{
}

allocator_boundary_tags::boundary_iterator::boundary_iterator(
    void* trusted)
    : _occupied_ptr(trusted),
      _occupied(false),
      _trusted_memory(trusted)
{
}

void* allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}