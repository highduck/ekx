#pragma once

#include <string>
#include <unordered_map>
#include <ek/util/common_macro.hpp>
#include <memory>
#include <utility>

namespace ek {

template<typename T>
class Res {

    struct slot : private disable_copy_assign_t {
        explicit slot(std::string key_) : key{std::move(key_)} {}

        const T* content = nullptr;

        // stored unique access key
        std::string key;
    };

    using slot_unique_ptr = std::unique_ptr<slot>;

    static slot* getSlotPointer(const std::string& id);

    static slot emptySlot;
    static std::unordered_map<std::string, slot_unique_ptr> storage_;

public:
    Res() : slot_{&emptySlot} {}

    explicit Res(const std::string& id);

    void setID(const std::string& id);

    [[nodiscard]]
    const std::string& getID() const;

    inline void reset(const T* res = nullptr) {
        if (slot_->content && slot_->content != res) {
            delete slot_->content;
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
        return slot_->content;
    }

    inline const T* get_or(const T* def) const {
        return slot_->content ? slot_->content : def;
    }

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

    inline static const std::unordered_map<std::string, slot_unique_ptr>& map() {
        return storage_;
    }

private:
    slot* slot_;
};

template<typename T>
std::unordered_map<std::string, typename Res<T>::slot_unique_ptr> Res<T>::storage_{};

template<typename T>
typename Res<T>::slot Res<T>::emptySlot{""};

template<typename T>
typename Res<T>::slot* Res<T>::getSlotPointer(const std::string& id) {
    slot_unique_ptr& ptr = storage_[id];
    if (!ptr) {
        ptr.reset(new slot(id));
    }
    return ptr.get();
}

template<typename T>
Res<T>::Res(const std::string& id)
        : slot_{getSlotPointer(id)} {
}

template<typename T>
void Res<T>::setID(const std::string& id) {
    slot_ = getSlotPointer(id);
}

template<typename T>
const std::string& Res<T>::getID() const {
    return slot_->key;
}

}