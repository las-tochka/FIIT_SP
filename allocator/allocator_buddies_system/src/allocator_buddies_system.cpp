#include <not_implemented.h>
#include <cstddef>
#include <algorithm>
#include <new>
#include "../include/allocator_buddies_system.h"

static inline size_t pow2(unsigned char k) noexcept
{
    return 1ull << k;
}

allocator_buddies_system::~allocator_buddies_system()
{
    std::lock_guard<std::mutex> lock(_mutex);
    
    if (!_trusted_memory)
        return;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    size_t total_size =
        *reinterpret_cast<size_t*>(mem);

    auto* parent =
        *reinterpret_cast<std::pmr::memory_resource**>(
            mem + sizeof(size_t));

    if (parent)
        parent->deallocate(_trusted_memory, total_size);
    else
        ::operator delete(_trusted_memory);

    _trusted_memory = nullptr;
}

allocator_buddies_system::allocator_buddies_system(
    allocator_buddies_system&& other) noexcept
{
    std::lock_guard<std::mutex> lock(other._mutex);
    _trusted_memory = other._trusted_memory;
    other._trusted_memory = nullptr;
}

allocator_buddies_system&
allocator_buddies_system::operator=(
    allocator_buddies_system&& other) noexcept
{
    if (this == &other)
        return *this;

    std::unique_lock<std::mutex> lock1(_mutex, std::defer_lock);
    std::unique_lock<std::mutex> lock2(other._mutex, std::defer_lock);
    std::lock(lock1, lock2);

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
    if (space_size < 2)
        throw std::logic_error("space_size too small");

    size_t real_size = 1;
    while (real_size < space_size)
        real_size <<= 1;

    space_size = real_size;

    _trusted_memory = parent_allocator
        ? parent_allocator->allocate(space_size)
        : ::operator new(space_size);

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    *reinterpret_cast<size_t*>(mem) = space_size;

    *reinterpret_cast<std::pmr::memory_resource**>(
        mem + sizeof(size_t)) = parent_allocator;

    *reinterpret_cast<fit_mode*>(
        mem + sizeof(size_t) + sizeof(void*)) = allocate_fit_mode;

    auto* root = reinterpret_cast<block_metadata*>(
        mem + allocator_metadata_size);

    root->occupied = false;

    root->size =
        static_cast<unsigned char>(
            __detail::nearest_greater_k_of_2(
                space_size - allocator_metadata_size));
}

[[nodiscard]] void* allocator_buddies_system::do_allocate_sm(size_t size)
{
    std::lock_guard<std::mutex> lock(_mutex);

    constexpr size_t MIN_ALLOCATION = sizeof(block_metadata) > 8 ? sizeof(block_metadata) : 8;

    if (size < MIN_ALLOCATION)
        size = MIN_ALLOCATION;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    size_t total_size =
        *reinterpret_cast<size_t*>(mem);
    
    size_t need = size + sizeof(block_metadata);

    unsigned char target_k =
        static_cast<unsigned char>(
            __detail::nearest_greater_k_of_2(need));

    // Получаем текущий режим выделения
    fit_mode current_mode = *reinterpret_cast<fit_mode*>(
        mem + sizeof(size_t) + sizeof(void*));

    char* begin = mem + allocator_metadata_size;
    char* end = mem + total_size;

    // Переменные для разных режимов
    char* best_fit_block = nullptr;
    unsigned char best_fit_size = UCHAR_MAX;
    
    char* worst_fit_block = nullptr;
    unsigned char worst_fit_size = 0;
    
    char* first_fit_block = nullptr;
    
    char* cur = begin;

    // Один проход для всех режимов
    while (cur < end)
    {
        auto* blk = reinterpret_cast<block_metadata*>(cur);

        if (blk->size == 0 || blk->size > 60)
            break;

        if (!blk->occupied && blk->size >= target_k)
        {
            // Для first fit - берем первый подходящий
            if (!first_fit_block)
                first_fit_block = cur;
            
            // Для best fit - ищем наименьший подходящий
            if (blk->size < best_fit_size)
            {
                best_fit_size = blk->size;
                best_fit_block = cur;
            }
            
            // Для worst fit - ищем наибольший подходящий
            if (blk->size > worst_fit_size)
            {
                worst_fit_size = blk->size;
                worst_fit_block = cur;
            }
        }

        cur += pow2(blk->size);
    }

    // Выбираем блок согласно режиму
    char* selected_block = nullptr;
    
    switch (current_mode)
    {
        case fit_mode::first_fit:
            selected_block = first_fit_block;
            break;
            
        case fit_mode::the_best_fit:
            selected_block = best_fit_block;
            break;
            
        case fit_mode::the_worst_fit:
            selected_block = worst_fit_block;
            break;
            
        default:
            selected_block = first_fit_block;
            break;
    }

    if (!selected_block)
        throw std::bad_alloc();

    auto* blk = reinterpret_cast<block_metadata*>(selected_block);
    
    // Разделяем блок до нужного размера
    while (blk->size > target_k)
    {
        unsigned char child_size =
            static_cast<unsigned char>(blk->size - 1);

        char* left_ptr = reinterpret_cast<char*>(blk);

        char* right_ptr =
            left_ptr + pow2(child_size);

        auto* left =
            reinterpret_cast<block_metadata*>(left_ptr);

        auto* right =
            reinterpret_cast<block_metadata*>(right_ptr);

        left->size = child_size;
        left->occupied = false;

        right->size = child_size;
        right->occupied = false;

        blk = left;
    }

    blk->occupied = true;

    return selected_block + sizeof(block_metadata);
}

void allocator_buddies_system::do_deallocate_sm(void* at)
{
    std::lock_guard<std::mutex> lock(_mutex);

    if (!at)
        return;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    size_t total_size =
        *reinterpret_cast<size_t*>(mem);

    char* begin = mem + allocator_metadata_size;
    char* end = mem + total_size;

    auto* blk =
        reinterpret_cast<block_metadata*>(
            reinterpret_cast<char*>(at)
            - sizeof(block_metadata));

    blk->occupied = false;

    char* block_ptr = reinterpret_cast<char*>(blk);

    while (true)
    {
        size_t block_size = (1ull << blk->size);

        size_t offset = block_ptr - begin;

        size_t buddy_offset = offset ^ block_size;

        char* buddy_ptr = begin + buddy_offset;

        if (buddy_ptr < begin || buddy_ptr >= end)
            break;

        auto* buddy =
            reinterpret_cast<block_metadata*>(buddy_ptr);

        if (buddy->occupied)
            break;

        if (buddy->size != blk->size)
            break;

        char* merged =
            std::min(block_ptr, buddy_ptr);

        blk = reinterpret_cast<block_metadata*>(merged);

        blk->occupied = false;
        blk->size++;

        block_ptr = merged;
    }
}

allocator_buddies_system::allocator_buddies_system(
    const allocator_buddies_system& other)
{
    std::lock_guard<std::mutex> lock(other._mutex);
    _trusted_memory = nullptr;

    throw std::logic_error(
        "copy not supported");
}

allocator_buddies_system&
allocator_buddies_system::operator=(
    const allocator_buddies_system& other)
{
    throw std::logic_error(
        "copy assignment not supported");
}

bool allocator_buddies_system::do_is_equal(
    const std::pmr::memory_resource& other) const noexcept
{
    std::lock_guard<std::mutex> lock(_mutex);
    return this == &other;
}

inline void allocator_buddies_system::set_fit_mode(
    allocator_with_fit_mode::fit_mode mode)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto* mem =
        reinterpret_cast<char*>(_trusted_memory);

    *reinterpret_cast<fit_mode*>(
        mem + fit_mode_offset) = mode;
}

std::vector<allocator_test_utils::block_info>
allocator_buddies_system::get_blocks_info() const noexcept
{
    std::lock_guard<std::mutex> lock(_mutex);
    return get_blocks_info_inner();
}
std::vector<allocator_test_utils::block_info>
allocator_buddies_system::get_blocks_info_inner() const
{
    std::vector<allocator_test_utils::block_info> res;

    if (!_trusted_memory)
        return res;

    auto* mem = reinterpret_cast<char*>(_trusted_memory);

    size_t total_size =
        *reinterpret_cast<size_t*>(mem);

    char* begin = mem + allocator_metadata_size;
    char* end = mem + total_size;

    char* cur = begin;

    while (cur < end)
    {
        auto* blk =
            reinterpret_cast<block_metadata*>(cur);

        if (blk->size == 0 || blk->size > 60)
            break;

        allocator_test_utils::block_info info;

        info.block_size = (1ull << blk->size);
        info.is_block_occupied = blk->occupied;

        res.push_back(info);

        cur += (1ull << blk->size);
    }

    return res;
}

allocator_buddies_system::buddy_iterator
allocator_buddies_system::begin() const noexcept
{
    std::lock_guard<std::mutex> lock(_mutex);
    return buddy_iterator(
        reinterpret_cast<char*>(_trusted_memory)
        + allocator_metadata_size);
}

allocator_buddies_system::buddy_iterator
allocator_buddies_system::end() const noexcept
{
    return buddy_iterator(nullptr);
}

bool allocator_buddies_system::buddy_iterator::operator==(
    const buddy_iterator& other) const noexcept
{
    return _block == other._block;
}

bool allocator_buddies_system::buddy_iterator::operator!=(
    const buddy_iterator& other) const noexcept
{
    return _block != other._block;
}

allocator_buddies_system::buddy_iterator&
allocator_buddies_system::buddy_iterator::operator++() & noexcept
{
    if (!_block)
        return *this;

    auto* blk =
        reinterpret_cast<block_metadata*>(_block);

    _block =
        reinterpret_cast<char*>(_block)
        + pow2(blk->size);

    return *this;
}

allocator_buddies_system::buddy_iterator
allocator_buddies_system::buddy_iterator::operator++(int)
{
    buddy_iterator tmp = *this;
    ++(*this);
    return tmp;
}

size_t allocator_buddies_system::buddy_iterator::size() const noexcept
{
    auto* blk =
        reinterpret_cast<block_metadata*>(_block);

    return pow2(blk->size);
}

bool allocator_buddies_system::buddy_iterator::occupied() const noexcept
{
    auto* blk =
        reinterpret_cast<block_metadata*>(_block);

    return blk->occupied;
}

void* allocator_buddies_system::buddy_iterator::operator*() const noexcept
{
    return _block;
}

allocator_buddies_system::buddy_iterator::buddy_iterator(
    void* start)
{
    _block = start;
}

allocator_buddies_system::buddy_iterator::buddy_iterator()
{
    _block = nullptr;
}