#pragma once

#include <functional>
#include "../ds/Array.hpp"

namespace ek {

// inspired by https://github.com/klmr/multifunction
template<typename ...Args>
class Signal {
public:

    using Listener = uint32_t;

    struct Slot {
        std::function<void(Args...)> fn;
        Listener id;
        bool once;
    };

    uint32_t _nextId = 0;
    Array<Slot> _slots{};

    template<typename Fn>
    Listener add(Fn listener) {
//        assert(!locked_);
        const auto id = _nextId++;
        _slots.emplace_back(Slot{static_cast<Fn&&>(listener), id, false});
        return id;
    }

    template<typename Fn>
    Listener once(Fn listener) {
//        assert(!locked_);
        const auto id = _nextId++;
        _slots.emplace_back(Slot{static_cast<Fn&&>(listener), id, true});
        return id;
    }

    bool remove(Listener id) {
        for (uint32_t i = 0; i < _slots._size; ++i) {
            auto& slot = _slots[i];
            if (slot.id == id) {
                _slots.eraseAt(i);
                return true;
            }
        }
        return false;
    }

    void emit(Args... args) {
        uint32_t i = 0;
        while (i < _slots.size()) {
            const auto& slot = _slots.get(i);
            // copy function
//            slot.fn.
            //printf("%sz", (uintptr_t)slot.fn.target());
            std::function<void(Args...)> fn = slot.fn;


            bool once = slot.once;
            fn(args...);
            if (once) {
                _slots.eraseAt(i);
                continue;
            }
            ++i;
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
        once(invocation);
        return *this;
    }

    inline auto& operator-=(Listener tk) {
        remove(tk);
        return *this;
    }

    inline void operator()(Args...args) {
        emit(args...);
    }

    Signal() = default;

    Signal(Signal&& mf) noexcept: _nextId{mf._nextId},
                                  _slots{std::move(mf._slots)} {

    }

    Signal(const Signal& mf) noexcept : _nextId{mf._nextId},
                                        _slots{mf._slots} {

    }

    Signal& operator=(Signal&& mf) noexcept {
        _nextId = mf._nextId;
        _slots = std::move(mf._slots);
        return *this;
    }

    Signal& operator=(const Signal& mf) noexcept {
        _nextId = mf._nextId;
        _slots = mf._slots;
        return *this;
    }
};

}


