#include "../include/allocator_boundary_tags.h"

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

    auto* first =
        reinterpret_cast<block_header*>(
            reinterpret_cast<char*>(_trusted_memory)
            + allocator_metadata_size);

    first->size_and_flags = space_size;
    first->prev = nullptr;
    first->next = nullptr;
    first->owner = this;
}

[[notiscard]] void *allocator_boundary_tags::do_allocate_sm(size_t size)
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
        if (it.occupied())
            continue;

        if (it.size() < required)
            continue;

        auto* current =
            reinterpret_cast<block_header*>(it.get_ptr());

        if (mode == allocator_with_fit_mode::fit_mode::first_fit)
        {
            best = current;
            break;
        }

        if (mode == allocator_with_fit_mode::fit_mode::the_best_fit)
        {
            if (!best || it.size() < block_size(best))
                best = current;
        }
        else // worst_fit
        {
            if (!best || it.size() > block_size(best))
                best = current;
        }
    }

    if (!best)
        throw std::bad_alloc();

    size_t full_size = block_size(best);
    size_t remainder = full_size - required;

    if (remainder > occupied_block_metadata_size)
    {
        auto* next =
            reinterpret_cast<block_header*>(
                reinterpret_cast<char*>(best) + required);

        next->size_and_flags = remainder;
        set_occupied(next, false);

        next->prev = best;
        next->next = best->next;
        next->owner = this;

        if (best->next)
            reinterpret_cast<block_header*>(best->next)->prev = next;

        best->next = next;
        set_size(best, required);
    }

    set_occupied(best, true);

    return reinterpret_cast<char*>(best)
        + occupied_block_metadata_size;
}

void allocator_boundary_tags::do_deallocate_sm(void* at)
{
    if (!at)
        return;

    auto mutex_ptr =
        reinterpret_cast<std::mutex*>(
            reinterpret_cast<char*>(_trusted_memory)
            + sizeof(std::pmr::memory_resource*)
            + sizeof(allocator_with_fit_mode::fit_mode)
            + sizeof(size_t));

    std::lock_guard guard(*mutex_ptr);

    auto* cur =
        reinterpret_cast<block_header*>(
            reinterpret_cast<char*>(at)
            - occupied_block_metadata_size);

    if (cur->owner != this)
        throw std::logic_error("foreign block");

    set_occupied(cur, false);

    auto* next = reinterpret_cast<block_header*>(cur->next);
    if (next && !occupied(next))
    {
        set_size(cur, block_size(cur) + block_size(next));
        cur->next = next->next;

        if (next->next)
            reinterpret_cast<block_header*>(next->next)->prev = cur;
    }

    auto* prev = reinterpret_cast<block_header*>(cur->prev);
    if (prev && !occupied(prev))
    {
        set_size(prev, block_size(prev) + block_size(cur));
        prev->next = cur->next;

        if (cur->next)
            reinterpret_cast<block_header*>(cur->next)->prev = prev;
    }
}

void allocator_boundary_tags::set_fit_mode(
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

std::vector<allocator_test_utils::block_info>
allocator_boundary_tags::get_blocks_info_inner() const
{
    std::vector<block_info> res;

    for (auto it = begin(); it != end(); ++it)
    {
        auto* h =
            reinterpret_cast<block_header*>(it.get_ptr());

        res.push_back({block_size(h), occupied(h)});
    }

    return res;
}

allocator_boundary_tags&
allocator_boundary_tags::operator=(allocator_boundary_tags&& other) noexcept
{
    if (this == &other)
        return *this;

    void* log_old_this_memory  = _trusted_memory;
    void* log_old_other_memory = other._trusted_memory;

    this->~allocator_boundary_tags();

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}

bool allocator_boundary_tags::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept
{
    return this == &other;
}

bool allocator_boundary_tags::boundary_iterator::operator==(
    const allocator_boundary_tags::boundary_iterator& other) const noexcept
{
    return _occupied_ptr == other._occupied_ptr;
}

bool allocator_boundary_tags::boundary_iterator::operator!=(
    const boundary_iterator& other) const noexcept
{
    return _occupied_ptr != other._occupied_ptr;
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
    boundary_iterator copy = *this;
    ++(*this);
    return copy;
}

allocator_boundary_tags::boundary_iterator
allocator_boundary_tags::boundary_iterator::operator--(int)
{
    boundary_iterator copy = *this;
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

allocator_boundary_tags::boundary_iterator::boundary_iterator(void* trusted)
    : _occupied_ptr(trusted),
      _occupied(false),
      _trusted_memory(trusted)
{
}

void* allocator_boundary_tags::boundary_iterator::get_ptr() const noexcept
{
    return _occupied_ptr;
}