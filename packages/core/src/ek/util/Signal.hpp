#pragma once

#include <functional>
#include <utility>
#include "../ds/Array.hpp"
#include "../ds/FixedArray.hpp"
#include <ek/assert.h>
#include <ek/hash.h>

namespace ek {

// inspired by https://github.com/klmr/multifunction
template<typename ...Args>
class Signal {
public:

    class Function : public std::function<void(Args...)> {
    public:
//        Function() = default;
//        Function(const Function& fn) = delete;
//        Function& operator=(const Function& fn) = delete;
//        Function(Function&& fn) noexcept = default;
//        Function& operator=(Function&& fn) noexcept = default;
    };

    using Listener = uint32_t;

    struct Slot {
        Function fn;
        string_hash_t type = 0;
        Listener id = 0;
        bool once = false;
    };

    Array<Slot> _slots;
    uint32_t _nextId = 0;
    FixedArray<Slot, 64>* _acc = nullptr;

    template<typename Fn>
    Listener push(Fn&& listener, string_hash_t type, bool once) {
        const auto id = _nextId++;
        Slot s{Function{listener}, type, id, once};
        if (_acc) {
            _acc->emplace_back(std::move(s));
        } else {
            _slots.emplace_back(std::move(s));
        }
        return id;
    }

    template<typename Fn>
    Listener add(Fn&& listener, string_hash_t type = 0) {
        return push(listener, type, false);
    }

    template<typename Fn>
    Listener once(Fn&& listener, string_hash_t type = 0) {
        return push(listener, type, true);
    }

    bool remove(Listener id) {
        for (uint32_t i = 0; i < _slots.size(); ++i) {
            auto& slot = _slots[i];
            if (slot.id == id) {
                _slots.erase_at(i);
                return true;
            }
        }
        return false;
    }

    void emit_(string_hash_t type, Args... args) {
        FixedArray<Slot, 64> acc;
        _acc = &acc;
        uint32_t i = 0;
        while (i < _slots.size()) {
            const auto& slot = _slots.get(i);
            // copy function
//            slot.fn.
            //printf("%sz", (uintptr_t)slot.fn.target());
            if (type && type != slot.type) {
                ++i;
                continue;
            }

            const bool once = slot.once;
            if (once) {
                std::move(slot.fn)(args...);
                _slots.erase_at(i);
                continue;
            }
            slot.fn(args...);
            ++i;
        }
        _acc = nullptr;
        if (acc.size() > 0) {
            for (auto& s: acc) {
                _slots.emplace_back(std::move(s));
            }
        }
    }

    void emit(Args... args) {
        emit_(0, args...);
    }

    void clear() {
        _slots.clear();
    }

    template<typename Fn>
    auto& operator+=(Fn&& invocation) {
        push(invocation, 0, false);
        return *this;
    }

    template<typename Fn>
    auto& operator<<(Fn&& invocation) {
        push(invocation, 0, true);
        return *this;
    }

    auto& operator-=(Listener tk) {
        remove(tk);
        return *this;
    }

    void operator()(Args...args) {
        emit_(0, args...);
    }

    constexpr Signal() noexcept {
        ek_core_dbg_inc(EK_CORE_DBG_SIGNAL);
    }

    constexpr Signal(Signal&& mf) noexcept: _slots{std::move(mf._slots)},
                                            _nextId{mf._nextId},
                                            _acc{mf._acc} {
        ek_core_dbg_inc(EK_CORE_DBG_SIGNAL);
        mf._acc = nullptr;
    }

    Signal(const Signal& mf) noexcept: _slots{mf._slots},
                                       _nextId{mf._nextId} {
        ek_core_dbg_inc(EK_CORE_DBG_SIGNAL);
    }

    Signal& operator=(Signal&& mf) noexcept {
        _slots = std::move(mf._slots);
        _nextId = mf._nextId;
        _acc = mf._acc;
        mf._acc = nullptr;
        return *this;
    }

    Signal& operator=(const Signal& mf) noexcept {
        if (this != &mf) {
            _slots = mf._slots;
            _nextId = mf._nextId;
        }
        return *this;
    }

    ~Signal() noexcept {
        ek_core_dbg_dec(EK_CORE_DBG_SIGNAL);
    }
};

}


