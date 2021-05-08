#pragma once

#include <cstdint>
#include <cstddef>
#include <new>
#include "config.hpp"

#define EK_SIZEOF_U32(Type) static_cast<uint32_t>(sizeof(Type))
#define EK_ALIGNOF_U32(Type) static_cast<uint32_t>(alignof(Type))

namespace ek {

#ifdef EK_ALLOCATION_TRACKER

class AllocationTracker;

#endif

struct AllocatorStats {
    const char* label = nullptr;
    uint32_t index = 0;
    uint64_t span = 0;
    enum {
        Current = 0,
        Peak = 1,
        AllTime = 2
    };
    uint32_t allocations[3]{};
    uint32_t memoryAllocated[3]{};
    uint32_t memoryEffective[3]{};
};

class Allocator {
public:
#ifdef EK_ALLOCATION_TRACKER
    AllocationTracker* tracker = nullptr;
#endif
    const char* label = nullptr;
    Allocator* next = nullptr;
    Allocator* children = nullptr;

    void addChild(Allocator& child);

    void removeChild(Allocator& child);

    AllocatorStats getStatistics() const;

    explicit Allocator(const char* label_);

    virtual ~Allocator();

    virtual void* alloc(uint32_t size, uint32_t align) = 0;

    virtual void dealloc(void* ptr) = 0;

    template<typename T, typename ...Args>
    T* create(Args&& ...args);

    template<typename T>
    void destroy(T* ptr);

    void* reallocate(void* ptr, uint32_t oldSizeToCopy, uint32_t newSize, uint32_t align);

    template<typename T>
    [[nodiscard]] inline T* allocBufferForArray(uint32_t count) {
        return static_cast<T*>(alloc(EK_SIZEOF_U32(T) * count, EK_ALIGNOF_U32(T)));
    }
};

class SystemAllocator;

class AlignedAllocator;

class ProxyAllocator;

namespace memory {

void initialize();

void shutdown();

void clear(void* ptr, uint32_t size);

void copy(void* dest, const void* src, uint32_t size);

void* reallocate(Allocator& allocator, void* ptr, uint32_t oldSizeToCopy, uint32_t newSize, uint32_t align);

extern ProxyAllocator& stdAllocator;
extern AlignedAllocator& alignedAllocator;
extern SystemAllocator systemAllocator;

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

class SystemAllocator : public Allocator {
public:
    explicit SystemAllocator(const char* label_) noexcept;

    ~SystemAllocator() override;

    void* alloc(uint32_t size, uint32_t align) override;

    void dealloc(void* ptr) override;
};

class AlignedAllocator : public Allocator {
public:
    Allocator& allocator;
    const char* label;

    AlignedAllocator(Allocator& allocator, const char* label_) noexcept;

    ~AlignedAllocator() override;

    void* alloc(uint32_t size, uint32_t align) override;

    void dealloc(void* ptr) override;
};

class ProxyAllocator : public Allocator {
public:
    Allocator& allocator;

    ProxyAllocator(Allocator& allocator, const char* label_) noexcept;

    // proxy global std allocator
    explicit ProxyAllocator(const char* heapLabel) noexcept;

    ~ProxyAllocator() override;

    void* alloc(uint32_t size, uint32_t align) override;

    void dealloc(void* ptr) override;
};

uint32_t readAllocationMap(Allocator& allocator, uint64_t* rle, uint32_t blockSize);

}