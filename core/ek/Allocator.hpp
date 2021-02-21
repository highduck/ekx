#pragma once

#include <cstdint>
#include <cstddef>

namespace ek {

class Allocator {
public:
    virtual ~Allocator() = default;

    virtual void* alloc(size_t size, size_t align) = 0;

    virtual void dealloc(void* ptr) = 0;

    virtual void* realloc(void* ptr, size_t newSize, size_t align) = 0;
};

class StdAllocator : public Allocator {
public:
    const char* label;

    explicit StdAllocator(const char* label_);
    ~StdAllocator() override;

    void* alloc(size_t size, size_t align) override;

    void dealloc(void* ptr) override;

    void* realloc(void* ptr, size_t newSize, size_t align)  override;

    static StdAllocator global;
};

void* std_alloc(size_t size, size_t align = 4);

void std_free(void* ptr);

void* std_realloc(void* ptr, size_t newSize, size_t align = 4);

void* std_alloc_zero(size_t size);

}

#define EK_MALLOC(size) ::ek::std_alloc(size)
#define EK_REALLOC(ptr, size) ::ek::std_realloc(ptr, size)
#define EK_FREE(ptr) ::ek::std_free(ptr)
#define EK_ALLOC_ZERO(size) ::ek::std_alloc_zero(size)