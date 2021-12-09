#pragma once

#include <ek/assert.h>
#include "Type.hpp"

namespace ek::Locator {

// note: recommended to delete copy/move on all Service types used in Locator api

// note: if not found you will use NULL and automatically crash, so don't matter to make api hard and provide ref and pointer
template<typename T>
T* get();

template<typename T>
T& ref();

template<typename T, typename Impl = T, typename... Args>
T& create(Args&& ...args);

template<typename T, typename Impl = T>
void reset(Impl* value);

// noop assert if it's was not initialized before (it's totally fine)
template<typename T>
void destroy();

/** templated implementation **/

template<typename T>
struct ServiceStorage {
    ServiceStorage() = delete;

    ~ServiceStorage() = delete;

    static T* value;
};

template<typename T>
inline T* ServiceStorage<T>::value = nullptr;

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
    S::value = new Impl(args...);
    return *S::value;
}

template<typename T, typename Impl>
void reset(Impl* value) {
    using S = ServiceStorage<T>;
    EK_ASSERT(S::value == nullptr);
    S::value = value;
}

// noop assert if it was not initialized before (it's totally fine)
template<typename T>
void destroy() {
    using S = ServiceStorage<T>;
    if (S::value) {
        delete S::value;
    }
    S::value = nullptr;
}

}
