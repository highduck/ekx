#pragma once

#include <cstdint>
#include "../assert.hpp"
#include "../Allocator.hpp"

namespace ek {

class BitVector {
public:
    constexpr static uint32_t bits_per_element = 8u;
    constexpr static uint32_t bit_shift = 3u;
    constexpr static uint32_t bit_mask = 0x7u;
    using size_type = uint32_t;
    using data_type = uint8_t;

    class reference {
        friend class BitVector;

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

    explicit BitVector(size_type size, Allocator& allocator = memory::stdAllocator) :
            _allocator{allocator},
            _size{size},
            _len{(size >> bit_shift) + 1u} {
        _data = (uint8_t*) _allocator.alloc(_len, sizeof(void*));
        memory::clear(_data, _len);
    }

    ~BitVector() {
        _allocator.dealloc(_data);
    }

    inline void enable(size_type index) {
        EK_ASSERT(_data != nullptr && index < _size);
        _data[address(index)] |= mask(index);
    }

    inline void disable(size_type index) {
        EK_ASSERT(_data != nullptr && index < _size);
        _data[address(index)] &= ~mask(index);
    }

    inline bool get(size_type index) const {
        EK_ASSERT(_data != nullptr && index < _size);
        return (_data[address(index)] & mask(index)) != 0u;
    }

    inline void set(size_type index, bool value) {
        EK_ASSERT(_data != nullptr && index < _size);
        value ? enable(index) : disable(index);
    }

    inline bool operator[](size_type index) const {
        EK_ASSERT(_data != nullptr && index < _size);
        return get(index);
    }

    inline reference operator[](size_type index) {
        EK_ASSERT(_data != nullptr && index < _size);
        return reference{_data[address(index)], mask(index)};
    }

    inline bool is_false(size_type index) const {
        EK_ASSERT(_data != nullptr && index < _size);
        return (_data[address(index)] & mask(index)) == 0u;
    }

    inline bool is_true(size_type index) const {
        EK_ASSERT(_data != nullptr && index < _size);
        return (_data[address(index)] & mask(index)) != 0u;
    }

    inline bool enable_if_not(size_type index) {
        EK_ASSERT(_data != nullptr && index < _size);
        auto a = address(index);
        auto m = mask(index);
        auto v = _data[a];
        if ((v & m) == 0) {
            _data[a] = v | m;
            return true;
        }
        return false;
    }

    inline size_type size() const {
        return _size;
    }

    inline const uint8_t* data() const {
        return _data;
    }

    inline uint8_t* data() {
        return _data;
    }

private:

    Allocator& _allocator;
    uint8_t* _data;
    size_type _size;
    size_type _len;

    inline static size_type address(size_type index) {
        return index >> bit_shift;
    }

    inline static data_type mask(size_type index) {
        return 0x1u << (index & bit_mask);
    }
};

}