#include <not_implemented.h>
#include "../include/allocator_global_heap.h"

allocator_global_heap::allocator_global_heap() = default; // стандартные

allocator_global_heap::~allocator_global_heap() = default; // мьютект работает сам, доп настройка не требуется

allocator_global_heap::allocator_global_heap(const allocator_global_heap& other)
{
    // ничего копировать не нужно, да и в нем нет смысла
}

allocator_global_heap& allocator_global_heap::operator=(const allocator_global_heap& other)
{
    if (this != &other)
    {
        // ничего копировать не нужно
    }
    return *this;
}

allocator_global_heap::allocator_global_heap(allocator_global_heap&& other) noexcept
{
    // ничего переносить не нужно
}

allocator_global_heap& allocator_global_heap::operator=(allocator_global_heap&& other) noexcept
{
    if (this != &other)
    {
        // ничего переносить не нужно
    }
    return *this;
}

void* allocator_global_heap::do_allocate_sm(size_t size)
{
    std::lock_guard<std::mutex> lock(mtx); // захват мьютекса

    if (size == 0) // 0 невалиден и непредскажуем, зависит от компилятора
        size = 1;

    return ::operator new(size); // при ошибке вернет std::bad_alloc и с ним тоже можно работать, точнее обработать на след этапе
    // мьютекс освобождается автоматически
}

// 

void allocator_global_heap::do_deallocate_sm(void* at)
{
    std::lock_guard<std::mutex> lock(mtx); // ф-ция автоматическая освобождать не надо

    if (at == nullptr)
        return;

    ::operator delete(at);
}

bool allocator_global_heap::do_is_equal(const std::pmr::memory_resource& other) const noexcept
{
    return dynamic_cast<const allocator_global_heap*>(&other) != nullptr;
}
