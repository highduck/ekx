#pragma once

#include <cstdint>
#include <cstddef>
#include <new>
#include "config.hpp"

namespace ek {

#ifdef EK_ALLOCATION_TRACKER
class AllocationTracker;
#endif

class Allocator {
public:
#ifdef EK_ALLOCATION_TRACKER
    AllocationTracker* tracker = nullptr;
#endif

    virtual ~Allocator() = default;

    virtual void* alloc(uint32_t size, uint32_t align) = 0;

    virtual void dealloc(void* ptr) = 0;

    template<typename T, typename ...Args>
    T* create(Args&& ...args);

    template<typename T>
    void destroy(T* ptr);

    void* reallocate(void* ptr, uint32_t oldSizeToCopy, uint32_t newSize, uint32_t align);
};

class StdAllocator;

class ProxyAllocator;

namespace memory {

void initialize();

void shutdown();

void clear(void* ptr, uint32_t size);

void copy(void* dest, const void* src, uint32_t size);

void* reallocate(Allocator& allocator, void* ptr, uint32_t oldSizeToCopy, uint32_t newSize, uint32_t align);

extern StdAllocator& stdAllocator;

class Globals;

}

inline void* Allocator::reallocate(void* ptr, uint32_t oldSizeToCopy, uint32_t newSize, uint32_t align) {
    return memory::reallocate(*this, ptr, oldSizeToCopy, newSize, align);
}

template<typename T, typename... Args>
inline T* Allocator::create(Args&& ...args) {
    void* memory = alloc(sizeof(T), alignof(T));
    return new(memory) T(std::forward<Args&&>(args)...);
}

template<typename T>
inline void Allocator::destroy(T* ptr) {
    ptr->~T();
    dealloc(ptr);
}

class StdAllocator : public Allocator {
    friend class memory::Globals;

protected:

    StdAllocator() noexcept;

    ~StdAllocator() override;

public:

    void* alloc(uint32_t size, uint32_t align) override;

    void dealloc(void* ptr) override;

    void* realloc(void* ptr, uint32_t newSize);
};

class ProxyAllocator : public Allocator {
public:
    Allocator& allocator;
    const char* label;

    ProxyAllocator(Allocator& allocator, const char* label_) noexcept;

    // proxy global std allocator
    explicit ProxyAllocator(const char* heapLabel) noexcept;

    ~ProxyAllocator() override;

    void* alloc(uint32_t size, uint32_t align) override;

    void dealloc(void* ptr) override;
};

}