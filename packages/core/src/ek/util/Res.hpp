#pragma once

#include <ek/ds/String.hpp>
#include <ek/util/Type.hpp>
#include <unordered_map>
#include "StaticStorage.hpp"

namespace ek::ResourceDB {

struct Key final {
    int type = 0;
    String name{};

    constexpr bool operator==(const Key& other) const noexcept {
        return type == other.type && name == other.name;
    }

    [[nodiscard]]
    constexpr uint64_t hash() const noexcept {
        return type ^ name.hash();
    }
};
}

template<>
struct std::hash<ek::ResourceDB::Key> {
    std::size_t operator()(const ek::ResourceDB::Key& s) const noexcept {
        return (std::size_t) s.hash();
    }
};

namespace ek {

namespace ResourceDB {

struct Slot final {
    // stored unique access key
    Key key;
    void* content = nullptr;

    Slot() = default;

    Slot(const Slot& v) = delete;

    Slot& operator=(const Slot& v) = delete;

    Slot(Slot&& v) = default;

    Slot& operator=(Slot&& v) = default;
};

struct DB {

    std::unordered_map<Key, Slot> map;
    Slot empty;

    Slot* getSlotPointer(Key&& key) {
        auto it = map.find(key);
        if (it == map.end()) {
            auto& s = map[key];
            s.key = key;
            return &s;
        }
        return &it->second;
    }
};

inline StaticStorage<DB> instance{};

inline Slot* getSlotPointer(int type, const String& name) {
    return instance.get().getSlotPointer({type, name});
}

inline Slot* getSlotPointer(int type, const char* name) {
    return  instance.get().getSlotPointer({type, String{name}});
}

inline Slot* getEmptySlot() {
    return &instance.get().empty;
}

}

template<typename T>
class Res {
public:
    constexpr Res() noexcept : slot_{ResourceDB::getEmptySlot()} {}

    explicit Res(const char* id);

    explicit Res(const String& id);

    void setID(const String& id);

    [[nodiscard]]
    const char* getID() const;

    inline void reset(T* res = nullptr) {
        if (slot_->content && slot_->content != res) {
            delete ((T*) slot_->content);
        }
        slot_->content = res;
    }

    // assign value-type via copy
    inline void assign(const T& res) {
        T* pRes = new T();
        *pRes = res;
        reset(pRes);
    }

    inline const T* get() const {
        return (const T*) slot_->content;
    }

    inline const T* get_or(const T* def) const {
        return slot_->content ? (const T*) slot_->content : def;
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
    ResourceDB::Slot* slot_;
};

template<typename T>
Res<T>::Res(const String& id) :
        slot_{ResourceDB::getSlotPointer(TypeIndex<T>::value, id)} {
}

template<typename T>
Res<T>::Res(const char* id) :
        slot_{ResourceDB::getSlotPointer(TypeIndex<T>::value, id)} {
}

template<typename T>
void Res<T>::setID(const String& id) {
    slot_ = ResourceDB::getSlotPointer(TypeIndex<T>::value, id);
}

template<typename T>
const char* Res<T>::getID() const {
    return slot_->key.name.c_str();
}

}