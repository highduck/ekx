#include "Allocator.hpp"
#include <cstdlib>
#include <Tracy.hpp>

namespace ek {

StdAllocator StdAllocator::global("global");

StdAllocator::StdAllocator(const char* label_) : label{label_} {
}

StdAllocator::~StdAllocator() = default;

void* StdAllocator::alloc(size_t size, size_t align) {
    void* ptr = ::malloc(size);
    TracyAllocN(ptr, size, label);
    return ptr;
}

void StdAllocator::dealloc(void* ptr) {
    TracyFreeN(ptr, label);
    ::free(ptr);
}

void* StdAllocator::realloc(void* ptr, size_t newSize, size_t align) {
    TracyFreeN(ptr, label);
    void* newPtr = ::realloc(ptr, newSize);
    TracyAllocN(newPtr, newSize, label);
    return newPtr;
}

void* std_alloc(size_t size, size_t align) {
    return StdAllocator::global.alloc(size, align);
}

void std_free(void* ptr) {
    StdAllocator::global.dealloc(ptr);
}

void* std_realloc(void* ptr, size_t newSize, size_t align) {
    return StdAllocator::global.realloc(ptr, newSize, align);
}

void clearMemory(void* ptr, size_t size) {
    char* it = static_cast<char*>(ptr);
    const char* end = it + size;
    while (it != end) {
        *(it++) = 0;
    }
}

void* std_alloc_zero(size_t size) {
    void* ptr = StdAllocator::global.alloc(size, 4);
    clearMemory(ptr, size);
    return ptr;
}

}