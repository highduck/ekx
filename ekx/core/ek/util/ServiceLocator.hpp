#pragma once

#include "../assert.hpp"
#include "../Allocator.hpp"

namespace ek::Locator {

void setup();

// note: recommended to delete copy/move on all Service types used in Locator api

// note: if not found you will use NULL and automatically crash, so don't matter to make api hard and provide ref and pointer
template<typename T>
T* get();

template<typename T>
T& ref();

template<typename T, typename Impl = T, typename... Args>
T& create(Args&& ...args);

// noop assert if it's was not initialized before (it's totally fine)
template<typename T>
void destroy();

/** templated implementation **/

inline Allocator* allocator = nullptr;

template<typename T>
struct ServiceStorage {
    ServiceStorage() = delete;

    ~ServiceStorage() = delete;

    static T* value;
};

template<typename T>
T* ServiceStorage<T>::value = nullptr;

template<typename T>
T* get() {
    return ServiceStorage<T>::value;
}

template<typename T>
T& ref() {
    return *ServiceStorage<T>::value;
}

template<typename T, typename Impl, typename... Args>
T& create(Args&& ...args) {
    using S = ServiceStorage<T>;
    EK_ASSERT(S::value == nullptr);
    EK_ASSERT(allocator != nullptr);
    S::value = allocator->create<Impl>(args...);
    return *S::value;
}

// noop assert if it's was not initialized before (it's totally fine)
template<typename T>
void destroy() {
    using S = ServiceStorage<T>;
    if (S::value && allocator) {
        allocator->destroy(S::value);
    }
    S::value = nullptr;
}

inline void setup() {
    if (!allocator) {
        allocator = memory::systemAllocator.create<ProxyAllocator>(memory::systemAllocator, "Locator");
    }
}

}
