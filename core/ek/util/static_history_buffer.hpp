#pragma once

namespace ek {

template<typename T, int Capacity>
class static_history_buffer {
public:
    void write(const T& v) {
        buf_[(idx_write_++) % Capacity] = v;
    }

    inline bool empty() const {
        return false;
    }

    inline int size() const {
        return Capacity;
    }

    inline int capacity() const {
        return Capacity;
    }

    // 0 1 2 3
    //   ^   ^
    //   |   1 + 2 = 3
    //   1 + 0 = 1
    // ^
    // 1 + 3 = 4 % 4 = 0
    //   1 + 4 = 5 % 4 = 1

    inline T at(int index) const {
        return buf_[(idx_write_ + index) % Capacity];
    }

    inline T at_backward(int index) const {
        return buf_[(Capacity + idx_write_ - 1 - (index % Capacity)) % Capacity];
    }

private:
    int idx_write_{0};
    T buf_[Capacity]{};
};
}


