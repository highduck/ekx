#pragma once

#include <cstdint>
#include <cstddef>
#include <new>
#include "config.hpp"

#define EK_SIZEOF_U32(Type) static_cast<uint32_t>(sizeof(Type))
#define EK_ALIGNOF_U32(Type) static_cast<uint32_t>(alignof(Type))

namespace ek {

class Allocator;

struct AllocationInfo {
    uintptr_t id = 0;
    uint32_t sizeUsed = 0;
    uint32_t sizeTotal = 0;
    inline static constexpr unsigned MaxStackDepth = 3;
    const char* stack[MaxStackDepth]{};
};

class DebugRecords {
public:
    constexpr inline static uint32_t MaxCount = 10000;
    AllocationInfo data[MaxCount];
    uint32_t size = 0;

    bool add(AllocationInfo rec) {
        if (size < MaxCount) {
            data[size++] = rec;
            return true;
        }
        return false;
    }

    bool remove(uintptr_t id) {
        return removeAt(findIndex(id));
    }

    bool removeAt(uint32_t index) {
        if (index < size) {
            if (index != size - 1) {
                data[index] = data[size - 1];
            }
            --size;
            return true;
        }
        return false;
    }

    [[nodiscard]]
    uint32_t findIndex(uintptr_t id) const {
        for (uint32_t i = 0; i < size; ++i) {
            if (data[i].id == id) {
                return i;
            }
        }
        return 0xFFFFFFFFu;
    }

    [[nodiscard]]
    const AllocationInfo* find(uintptr_t id) const {
        const auto index = findIndex(id);
        return index < size ? &data[index] : nullptr;
    }

    constexpr inline static uint32_t LabelStackMaxCount = 100;
    inline static const char* labelsStack[LabelStackMaxCount]{};
    inline static uint32_t labelsSize = 0;

    static void pushDebugLabel(const char* label) {
        if (labelsSize < LabelStackMaxCount) {
            labelsStack[labelsSize++] = label;
        }
    }

    static void popDebugLabel() {
        if (labelsSize > 0) {
            --labelsSize;
        } else {
            // assert
        }
    }

    [[nodiscard]]
    static const char* getLabel() {
        const char* result = nullptr;
        if (labelsSize > 0) {
            result = labelsStack[labelsSize - 1];
        }
        return result;
    }
};

struct AllocatorTraceScope {
    explicit AllocatorTraceScope(const char* label) {
        DebugRecords::pushDebugLabel(label);
    }

    ~AllocatorTraceScope() {
        DebugRecords::popDebugLabel();
    }
};

struct AllocatorStats {
    const char* label = nullptr;
    uint32_t index = 0;
    enum {
        Current = 0,
        Peak = 1,
        AllTime = 2
    };
    uint32_t allocations[3]{};
    uint32_t memoryAllocated[3]{};
    uint32_t memoryEffective[3]{};

    Allocator* next = nullptr;
    Allocator* children = nullptr;
};

class Allocator {
public:
#ifdef EK_ALLOCATION_TRACKER
    AllocatorStats stats;
    DebugRecords records{};
#endif

    void addChild(Allocator& child);

    void removeChild(Allocator& child);

    [[nodiscard]]
    const AllocatorStats* getStats() const {
#ifdef EK_ALLOCATION_TRACKER
        return &stats;
#else
        return nullptr;
#endif
    }

    [[nodiscard]]
    uint64_t getSpanSize() const;

    static void pushDebugLabel(const char* label);

    static void popDebugLabel();

    uint32_t getAllocationsInfo(uint32_t maxCount, AllocationInfo* outData) const;

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
    return new(memory) T(/* forward */static_cast<Args&&>(args)...);
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