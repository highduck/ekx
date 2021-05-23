#pragma once

#include "../ds/Array.hpp"

namespace ek {

template<typename ...Args>
class StaticSignal {
public:
    typedef void(Function)(Args...);

    struct Slot {
        Function* fn;
        bool once;
    };

    inline void add(Function* listener) {
        _slots.emplace_back({listener, false});
    }

    inline void addOnce(Function* listener) {
        _slots.emplace_back({listener, true});
    }

    bool remove(Function* listener) {
        const unsigned end = _slots._size;
        for (unsigned i = 0; i < end; ++i) {
            if (_slots[i].fn == listener) {
                _slots.eraseAt(i);
                return true;
            }
        }
        return false;
    }

    void emit(Args... args) {
        unsigned i = 0;
        while (i < _slots.size()) {
            // copy slot
            Slot slot = _slots[i];
            slot.fn(args...);
            if (slot.once) {
                _slots.eraseAt(i);
            } else {
                ++i;
            }
        }
    }

    void clear() {
        _slots.clear();
    }

    inline auto& operator+=(Function* fn) {
        add(fn);
        return *this;
    }

    inline auto& operator<<(Function* fn) {
        addOnce(fn);
        return *this;
    }

    inline auto& operator-=(Function* fn) {
        remove(fn);
        return *this;
    }

    inline void operator()(Args...args) {
        return emit(args...);
    }

    StaticSignal() = default;

    StaticSignal(StaticSignal&& mf) noexcept = default;

    StaticSignal& operator=(StaticSignal&& mf) noexcept = default;

private:
    Array<Slot> _slots{memory::stdAllocator};
};

}


