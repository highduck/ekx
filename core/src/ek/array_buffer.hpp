#pragma once

#include <cstdint>
#include <cstring>
#include <cstddef>
#include <cassert>

namespace ek {

template<typename T>
class basic_array_buffer;

using array_buffer = basic_array_buffer<uint8_t>;

template<typename T>
class basic_array_buffer {
public:

    inline constexpr basic_array_buffer() noexcept
            : data_{nullptr},
              size_{0u} {

    }

    inline constexpr explicit basic_array_buffer(size_t size) noexcept
            : data_{new T[size]},
              size_{size} {

    }

    // COPY DATA
    inline constexpr basic_array_buffer(const T* data, size_t size) noexcept
            : data_{new T[size]},
              size_{size} {
        memcpy(data_, data, size);
    }

    inline constexpr basic_array_buffer(basic_array_buffer<T>&& m) noexcept
            : data_{m.data()},
              size_{m.size()} {
        m.reset();
    }

    inline ~basic_array_buffer() noexcept {
        release();
    }

    inline constexpr basic_array_buffer<T>& operator=(basic_array_buffer<T>&& m) noexcept {
        release();
        data_ = m.data_;
        size_ = m.size_;
        m.reset();
        return *this;
    }

    inline constexpr T* data() noexcept {
        return data_;
    }

    inline constexpr const T* data() const noexcept {
        return data_;
    }

    [[nodiscard]] inline constexpr size_t size() const noexcept {
        return size_;
    }

    [[nodiscard]] inline constexpr bool empty() const noexcept {
        return size_ == 0u;
    }

    inline T operator[](size_t index) const {
        assert(data_ != nullptr && index < size_);
        return data_[index];
    }

private:
    inline constexpr void release() noexcept {
        if (data_) {
            delete[] data_;
            data_ = nullptr;
        }
    }

    inline constexpr void reset(T* data = nullptr, size_t size = 0u) noexcept {
        data_ = data;
        size_ = size;
    }

private:
    T* data_;
    size_t size_;
};

}