#pragma once

#include "../assert.hpp"

namespace ek {

template<typename T, typename SizeType, SizeType Capacity>
class StaticRingBuffer {
public:

    void push(const T v) {
        EK_ASSERT(size() != capacity());
        buf_[(idx_write_++) % Capacity] = v;
    }

    T shift() {
        EK_ASSERT(size() != 0);
        return buf_[(idx_read_++) % Capacity];
    }

    [[nodiscard]] inline bool empty() const {
        return idx_write_ == idx_read_;
    }

    inline SizeType size() const {
        return idx_write_ - idx_read_;
    }

    inline SizeType capacity() const {
        return Capacity;
    }

private:
    SizeType idx_write_{0};
    SizeType idx_read_{0};
    T buf_[Capacity];
};

}