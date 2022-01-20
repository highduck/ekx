#pragma once

#include <ek/log.h>
#include <ek/hash.h>
#include <ek/ds/Array.hpp>
#include <ek/util/Type.hpp>

#include "StaticStorage.hpp"

namespace ek {

namespace ResourceDB {

struct Slot final {
    // stored unique access key
    string_hash_t name = 0;
    void* content = nullptr;

    Slot() = default;

    Slot(const Slot& v) = delete;

    Slot& operator=(const Slot& v) = delete;

    Slot(Slot&& v) = default;

    Slot& operator=(Slot&& v) = default;
};

inline StaticStorage<Array<Slot>[32]> s_map;

inline Array<Slot>& list(int type) {
    return *(s_map.get() + type);
}

inline uint32_t getSlotPointer(int type, string_hash_t name) {
#if 0
    static int maxsize = 0;
        if(maxsize < key.name.size()) {
            maxsize = key.name.size();
            log_debug("max size: %d '%s'", maxsize, key.name.c_str());
        }
#endif
    Array<Slot>& arr = list(type);
    uint32_t i = 0;
    while (i < arr.size()) {
        if (arr[i].name == name) {
            return i;
        }
        ++i;
    }
    arr.emplace_back({name, nullptr});
    return i;
}

inline uint32_t inject(int type, string_hash_t name) {
    return getSlotPointer(type, name);
}

inline void reset(int type, uint32_t handle, void* r, void(* deleter)(void*)) {
    Slot& slot = list(type)[handle];
    if (slot.content && slot.content != r) {
        deleter(slot.content);
    }
    slot.content = r;
}

inline Slot* get(int type, uint32_t handle) {
    return list(type).begin() + handle;
}

inline void* getContent(int type, uint32_t handle) {
    return get(type, handle)->content;
}

inline const char* getName(int type, uint32_t handle) {
    const string_hash_t name = get(type, handle)->name;
    const char* str = hsp_get(name);
    return str ? str : "";
}

inline void init() {
    s_map.initialize();
}

}

template<typename T>
class Res {
public:
    constexpr Res() noexcept: h{ResourceDB::inject(TypeIndex<T>::value, 0)} {}

    explicit Res(string_hash_t id);

    void setID(string_hash_t id);

    // WARNING: please use it only for debugging
    [[nodiscard]] const char* getID() const;

    static void deleter(void* obj) {
        delete (T*)obj;
    }

    inline void reset(T* res = nullptr) {
        ResourceDB::reset(TypeIndex<T>::value, h, res, Res<T>::deleter);
    }

    // assign value-type via copy
    inline void assign(const T& res) {
        T* pRes = new T();
        *pRes = res;
        reset(pRes);
    }

    inline const T* get() const {
        return (const T*) ResourceDB::getContent(TypeIndex<T>::value, h);
    }

    inline const T* get_or(const T* def) const {
        const T* c = get();
        return c ? c : def;
    }

    [[nodiscard]]
    inline bool empty() const {
        return !get();
    }

    inline const T& operator*() const {
        return *get();
    }

    inline const T* operator->() const {
        return get();
    }

    inline T* operator->() {
        return get_mutable();
    }

    inline explicit operator bool() const {
        return !empty();
    }

    inline explicit operator const T*() const {
        return get();
    }

    inline T* get_mutable() const {
        return const_cast<T*>(get());
    }

    inline T& mutableRef() const {
        auto& ref = *get();
        return const_cast<T&>(ref);
    }

private:
    uint32_t h;
};

template<typename T>
Res<T>::Res(string_hash_t id) :
        h{ResourceDB::inject(TypeIndex<T>::value, id)} {
}

template<typename T>
void Res<T>::setID(string_hash_t id) {
    h = ResourceDB::inject(TypeIndex<T>::value, id);
}

template<typename T>
const char* Res<T>::getID() const {
    return ResourceDB::getName(TypeIndex<T>::value, h);
}

}