#pragma once

#include <string>
#include <unordered_map>
#include <ek/util/common_macro.hpp>
#include <utility>

namespace ek {

template<typename T>
class asset_t {

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
    asset_t() : slot_{&emptySlot} {}

    explicit asset_t(const std::string& id);

    void setID(const std::string& id);

    [[nodiscard]]
    const std::string& getID() const;

    inline void reset(const T* res = nullptr) {
        if (slot_->content && slot_->content != res) {
            delete slot_->content;
        }
        slot_->content = res;
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

    inline static const std::unordered_map<std::string, slot_unique_ptr>& map() {
        return storage_;
    }

private:
    slot* slot_;
};

template<typename T>
std::unordered_map<std::string, typename asset_t<T>::slot_unique_ptr> asset_t<T>::storage_{};

template<typename T>
typename asset_t<T>::slot asset_t<T>::emptySlot{""};

template<typename T>
typename asset_t<T>::slot* asset_t<T>::getSlotPointer(const std::string& id) {
    slot_unique_ptr& ptr = storage_[id];
    if (!ptr) {
        ptr.reset(new slot(id));
    }
    return ptr.get();
}

template<typename T>
asset_t<T>::asset_t(const std::string& id)
        : slot_{getSlotPointer(id)} {
}

template<typename T>
void asset_t<T>::setID(const std::string& id) {
    slot_ = getSlotPointer(id);
}

template<typename T>
const std::string& asset_t<T>::getID() const {
    return slot_->key;
}

}