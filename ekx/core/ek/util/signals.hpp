#pragma once

#include <functional>
#include "../ds/Array.hpp"

namespace ek {

// inspired by https://github.com/klmr/multifunction

template<typename ...Args>
class signal_t {
public:

    using Listener = uint32_t;

    struct Slot {
        std::function<void(Args...)> fn;
        Listener id;
        bool once;
    };

    uint32_t _nextId = 0;
    Array<Slot> _slots;

    template<typename Fn>
    Listener add(Fn listener) {
//        assert(!locked_);
        const auto id = _nextId++;
        _slots.emplace_back(Slot{static_cast<Fn&&>(listener), id, false});
        return id;
    }

    template<typename Fn>
    Listener add_once(Fn listener) {
//        assert(!locked_);
        const auto id = _nextId++;
        _slots.emplace_back(Slot{static_cast<Fn&&>(listener), id, true});
        return id;
    }

    bool remove(Listener id) {
        for(uint32_t i = 0; i < _slots._size; ++i) {
            auto& slot = _slots[i];
            if(slot.id == id) {
                _slots.erase(i);
                return true;
            }
        }
        return false;
    }

    void emit(Args... args) {
        uint32_t i = 0;
        while (i < _slots._size) {
            // copy slot
            auto slot = _slots.get(i);
            slot.fn(args...);
            if (slot.once) {
                _slots.erase(i);
            } else {
                ++i;
            }
        }
    }

    void clear() {
        _slots.clear();
    }

    template<typename Fn>
    inline auto& operator+=(Fn invocation) {
        add(invocation);
        return *this;
    }

    template<typename Fn>
    inline auto& operator<<(Fn invocation) {
        add_once(invocation);
        return *this;
    }

    inline auto& operator-=(Listener tk) {
        remove(tk);
        return *this;
    }

    inline void operator()(Args...args) {
        emit(args...);
    }

    signal_t() = default;

    signal_t(signal_t&& mf) noexcept = default;

    signal_t(const signal_t& mf) noexcept = default;

    signal_t& operator=(signal_t&& mf) noexcept = default;

    signal_t& operator=(const signal_t& mf) noexcept = default;
};

}


