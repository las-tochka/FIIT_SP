#include <not_implemented.h>
#include <cstddef>
#include "../include/allocator_buddies_system.h"

allocator_buddies_system::~allocator_buddies_system()
{
    if (!_trusted_memory)
        return;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    auto parent = *reinterpret_cast<std::pmr::memory_resource**>(
        mem + sizeof(void*) + sizeof(fit_mode));

    size_t space = *reinterpret_cast<size_t*>(mem);

    if (parent)
        parent->deallocate(_trusted_memory, space);
    else
        ::operator delete(_trusted_memory);

    _trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system&& other) noexcept
{
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_buddies_system& allocator_buddies_system::operator=(
    allocator_buddies_system&& other) noexcept
{
    if (this == &other)
        return *this;

    this->~allocator_buddies_system();

    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;

    return *this;
}

allocator_buddies_system::allocator_buddies_system(
    size_t space_size,
    std::pmr::memory_resource* parent_allocator,
    allocator_with_fit_mode::fit_mode allocate_fit_mode)
{
    if (space_size == 0)
        throw std::logic_error("space_size must be greater than 0");

    if ((space_size & (space_size - 1)) != 0)
        throw std::logic_error("space_size must be a power of 2");

    std::pmr::memory_resource* parent = parent_allocator;

    _trusted_memory = parent
        ? parent->allocate(space_size)
        : ::operator new(space_size);

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    // header layout:
    *reinterpret_cast<std::pmr::memory_resource**>(mem)
        = parent;

    *reinterpret_cast<fit_mode*>(mem + sizeof(void*))
        = allocate_fit_mode;

    // root block metadata immediately after header
    auto* root = reinterpret_cast<block_metadata*>(
        mem + allocator_metadata_size);

    root->occupied = false;
    root->size = static_cast<unsigned char>(
        __detail::nearest_greater_k_of_2(
            space_size - allocator_metadata_size));
}

[[nodiscard]] void *allocator_buddies_system::do_allocate_sm(
    size_t size)
{
    std::lock_guard<std::mutex> lock(
        *reinterpret_cast<std::mutex*>(
            reinterpret_cast<char*>(_trusted_memory)
            + sizeof(void*)
            + sizeof(fit_mode)
            + sizeof(unsigned char)));

    if (size == 0)
        size = 1;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    fit_mode mode = *reinterpret_cast<fit_mode*>(
        mem + sizeof(void*));

    size_t need = size + sizeof(block_metadata);

    unsigned char target_k =
        static_cast<unsigned char>(
            __detail::nearest_greater_k_of_2(need));

    char* cur = mem + allocator_metadata_size;

    char* best = nullptr;
    size_t best_size = 0;

    while (cur < mem + allocator_metadata_size + (1ull << 20)) // safe bound
    {
        auto* blk = reinterpret_cast<block_metadata*>(cur);

        if (!blk || blk->size == 0 || blk->size > 30)
            continue;
        
        if (!blk->occupied && blk->size >= target_k)
        {
            size_t actual = 1ull << blk->size;

            if (mode == fit_mode::first_fit)
            {
                best = cur;
                break;
            }
            else if (mode == fit_mode::the_best_fit)
            {
                if (!best || actual < best_size)
                {
                    best = cur;
                    best_size = actual;
                }
            }
            else if (mode == fit_mode::the_worst_fit)
            {
                if (!best || actual > best_size)
                {
                    best = cur;
                    best_size = actual;
                }
            }
        }
}

    if (!best)
        throw std::bad_alloc();

    auto* blk = reinterpret_cast<block_metadata*>(best);

    blk->occupied = true;

    while (blk->size > target_k)
    {
        blk->size--;

    auto* buddy = reinterpret_cast<block_metadata*>(
        reinterpret_cast<char*>(best) + (1ull << blk->size));

        buddy->occupied = false;
        buddy->size = blk->size;
    }

    return best + sizeof(block_metadata);
}

void allocator_buddies_system::do_deallocate_sm(void* at)
{
    if (!at)
        return;

    auto* blk = reinterpret_cast<block_metadata*>(
        reinterpret_cast<char*>(at) - sizeof(block_metadata));

    blk->occupied = false;

    // buddy merge (linear scan)
    char* base = reinterpret_cast<char*>(blk);
    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    while (true)
    {
        size_t block_size = 1ull << blk->size;
        size_t offset = base - mem - allocator_metadata_size;

        size_t buddy_offset = offset ^ block_size;

        char* buddy_ptr = mem + allocator_metadata_size + buddy_offset;

        auto* buddy = reinterpret_cast<block_metadata*>(buddy_ptr);

        if (buddy_ptr < mem + allocator_metadata_size ||
            buddy_ptr >= mem + (1ull << 30))
            break;

        if (buddy->occupied || buddy->size != blk->size)
            break;

        blk = reinterpret_cast<block_metadata*>(
            std::min(base, buddy_ptr));

        blk->size++;
        base = reinterpret_cast<char*>(blk);
    }
}

allocator_buddies_system::allocator_buddies_system(const allocator_buddies_system &other)
{
    _trusted_memory = nullptr;

    throw std::logic_error(
        "copy of allocator_buddies_system is not supported");
}

allocator_buddies_system &allocator_buddies_system::operator=(const allocator_buddies_system &other)
{
    throw std::logic_error(
        "assignment of allocator_buddies_system is not supported");
}

bool allocator_buddies_system::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept
{
    return this == &other;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    *reinterpret_cast<fit_mode*>(mem + sizeof(void*)) = mode;
}


std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info() const noexcept
{
    return get_blocks_info_inner();
}

std::vector<allocator_test_utils::block_info> allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> res;

    if (!_trusted_memory)
        return res;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    auto* root = reinterpret_cast<block_metadata*>(
        mem + allocator_metadata_size);

    allocator_test_utils::block_info info;
    info.is_block_occupied = root->occupied;
    info.block_size = 1ull << root->size;

    res.push_back(info);

    return res;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::begin() const noexcept
{
    return buddy_iterator(
    reinterpret_cast<char*>(_trusted_memory)
    + allocator_metadata_size);
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::end() const noexcept
{
    return buddy_iterator(nullptr);
}

bool allocator_buddies_system::buddy_iterator::operator==(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(const allocator_buddies_system::buddy_iterator &other) const noexcept
{
    return _block != other._block;
}

allocator_buddies_system::buddy_iterator &allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    auto* blk = reinterpret_cast<block_metadata*>(_block);
    if (!_block)
        return *this;

    _block = reinterpret_cast<char*>(_block)
        + (1ull << blk->size);
        return *this;
}

allocator_buddies_system::buddy_iterator allocator_buddies_system::buddy_iterator::operator++(int n)
{
    buddy_iterator tmp = *this;
    ++(*this);
    return tmp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    auto* blk = reinterpret_cast<block_metadata*>(_block);
    return 1ull << blk->size;
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    auto* blk = reinterpret_cast<block_metadata*>(_block);
    return blk->occupied;
}

void *allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return _block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator(void *start)
{
    _block = start;
}

allocator_buddies_system::buddy_iterator::buddy_iterator()
{
    _block = nullptr;
}