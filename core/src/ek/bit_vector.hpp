#pragma once

#include <cstdint>
#include <cassert>
#include <algorithm>

namespace ek {

class bit_vector {
public:
    constexpr static uint32_t bits_per_element = 8u;
    constexpr static uint32_t bit_shift = 3u;
    constexpr static uint32_t bit_mask = 0x7u;
    using size_type = uint32_t;
    using data_type = uint8_t;

    class reference {
        friend class bit_vector;

    public:
        inline reference& operator=(bool value) {
            ptr_ = value ? (ptr_ | mask_) : (ptr_ & ~mask_);
            return *this;
        }

        inline operator bool() const {
            return (ptr_ & mask_) != 0u;
        }

    private:
        inline reference(data_type& ptr, data_type mask)
                : ptr_{ptr},
                  mask_{mask} {

        }

        data_type& ptr_;
        data_type mask_;
    };

    explicit bit_vector(size_type size)
            : size_{size},
              len_{(size >> bit_shift) + 1u},
              data_{new uint8_t[len_]} {
        std::fill_n(data_, len_, 0u);
    }

    ~bit_vector() {
        delete[] data_;
    }

    inline void enable(size_type index) {
        assert(data_ != nullptr && index < size_);
        data_[address(index)] |= mask(index);
    }

    inline void disable(size_type index) {
        assert(data_ != nullptr && index < size_);
        data_[address(index)] &= ~mask(index);
    }

    inline bool get(size_type index) const {
        assert(data_ != nullptr && index < size_);
        return (data_[address(index)] & mask(index)) != 0u;
    }

    inline void set(size_type index, bool value) {
        assert(data_ != nullptr && index < size_);
        value ? enable(index) : disable(index);
    }

    inline bool operator[](size_type index) const {
        assert(data_ != nullptr && index < size_);
        return get(index);
    }

    inline reference operator[](size_type index) {
        assert(data_ != nullptr && index < size_);
        return reference{data_[address(index)], mask(index)};
    }

    inline bool is_false(size_type index) const {
        assert(data_ != nullptr && index < size_);
        return (data_[address(index)] & mask(index)) == 0u;
    }

    inline bool is_true(size_type index) const {
        assert(data_ != nullptr && index < size_);
        return (data_[address(index)] & mask(index)) != 0u;
    }

    inline bool enable_if_not(size_type index) {
        assert(data_ != nullptr && index < size_);
        auto a = address(index);
        auto m = mask(index);
        auto v = data_[a];
        if ((v & m) == 0) {
            data_[a] = v | m;
            return true;
        }
        return false;
    }

    inline size_type size() const {
        return size_;
    }

    inline const uint8_t* data() const {
        return data_;
    }

    inline uint8_t* data() {
        return data_;
    }

private:

    size_type size_;
    size_type len_;
    uint8_t* data_;

    inline static size_type address(size_type index) {
        return index >> bit_shift;
    }

    inline static data_type mask(size_type index) {
        return 0x1u << (index & bit_mask);
    }
};

}