#pragma once

namespace ek {

template<typename T, typename SizeType, SizeType Capacity>
class static_history_buffer {
public:
    void write(const T& v) {
        buf_[(idx_write_++) % Capacity] = v;
    }

    inline bool empty() const {
        return false;
    }

    inline SizeType size() const {
        return Capacity;
    }

    inline SizeType capacity() const {
        return Capacity;
    }

    inline T at(SizeType index) const {
        return buf_[(idx_write_ + index) % Capacity];
    }

    inline T at_backward(SizeType index) const {
        return buf_[(Capacity + idx_write_ - 1 - (index % Capacity)) % Capacity];
    }

private:
    SizeType idx_write_{0};
    T buf_[Capacity]{};
};
}


