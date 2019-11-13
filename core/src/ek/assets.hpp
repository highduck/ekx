#pragma once

#include <string>
#include <unordered_map>
#include <ek/utility/common_macro.hpp>

namespace ek {

template<typename T>
class asset_t {

    struct slot : private disable_copy_assign_t {
        slot() = default;

        const T* content = nullptr;
    };

    static std::unordered_map<std::string, slot> storage_;

public:
    asset_t()
            : slot_{nullptr} {

    }

    explicit asset_t(const std::string& id);

    inline void reset(const T* res = nullptr) {
        if (slot_) {
            if (slot_->content) {
                delete slot_->content;
            }
            slot_->content = res;
        }
    }

    inline const T* get() const {
        return slot_ ? slot_->content : nullptr;
    }

    inline const T* get_or(const T* def) const {
        return slot_ ? slot_->content : def;
    }

    inline bool empty() const {
        return get() == nullptr;
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
        return slot_ && slot_->content;
    }

    inline explicit operator const T*() const {
        return get();
    }

    inline T* get_mutable() const {
        return const_cast<T*>(get());
    }

    inline static const std::unordered_map<std::string, slot>& map() {
        return storage_;
    }

private:
    slot* slot_ = nullptr;
};

template<typename T>
std::unordered_map<std::string, typename asset_t<T>::slot> asset_t<T>::storage_{};

template<typename T>
asset_t<T>::asset_t(const std::string& id)
        : slot_{&asset_t<T>::storage_[id]} {
}

}