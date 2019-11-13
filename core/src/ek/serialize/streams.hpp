#pragma once

#include "core.hpp"

#include <cstddef>
#include <cassert>
#include <cstring>
#include <utility>
#include <string>

// _LIBCPP_STD_VER <= 14
#if (__cplusplus <= 201402L)
namespace std {
    typedef unsigned char byte;
}
#endif

namespace ek {

namespace details {

template<typename Input, typename Size, typename Output>
inline void copy(const Input src, const Size size, Output dest) noexcept {
    std::memcpy(reinterpret_cast<void* __restrict__>(dest),
                reinterpret_cast<const void* __restrict__>(src),
                size);
}

}

class input_memory_stream {
public:

    input_memory_stream() = delete;

    template<typename SizeType = uint32_t>
    input_memory_stream(const void* data, SizeType size)
            : data_{static_cast<const std::byte*>(data)},
              size_{static_cast<uint32_t>(size)},
              pos_{0u} {
        assert(size < SizeType{0x1FFFFFFF});
    }

    input_memory_stream(const input_memory_stream&) = delete;

    input_memory_stream(input_memory_stream&&) = delete;

    input_memory_stream& operator=(input_memory_stream&&) = delete;

    input_memory_stream& operator=(const input_memory_stream&) = delete;

    inline uint32_t position() const {
        return pos_;
    }

    void position(uint32_t value) {
        assert(value <= size_);
        pos_ = value;
    }

    inline bool eos() const {
        return pos_ >= size_;
    }

    uint32_t seek(const int32_t offset) {
        assert(static_cast<int32_t>(pos_) + offset >= 0);
        assert(static_cast<int32_t>(pos_) + offset <= static_cast<int32_t>(size_));
        return pos_ += offset;
    }

    inline void read(void* dest, const uint32_t size) noexcept {
        assert(pos_ + size <= size_);
        assert(pos_ + size >= pos_); // check overflow
        details::copy(data_ + pos_, size, dest);
        pos_ += size;
    }

    template<typename T>
    inline void read(T& to_value) noexcept {
        constexpr uint32_t size = sizeof(T);
        assert(pos_ + size <= size_);
        details::copy(data_ + pos_, size, &to_value);
        pos_ += size;
    }

    template<typename T>
    inline T read() noexcept {
        T value;
        read(value);
        return value;
    }

    inline uint32_t size() const {
        return size_;
    }

    inline const void* data() const {
        return data_;
    }

private:
    const std::byte* data_;
    const uint32_t size_;
    uint32_t pos_;
};

class output_memory_stream {
public:

    output_memory_stream() = delete;

    explicit output_memory_stream(uint32_t capacity)
            : data_{new std::byte[capacity]},
              pos_{0u},
              cap_{capacity} {
    }

    ~output_memory_stream() {
        delete[] data_;
    }

    output_memory_stream(const output_memory_stream&) = delete;

    output_memory_stream(output_memory_stream&&) = delete;

    output_memory_stream& operator=(output_memory_stream&&) = delete;

    output_memory_stream& operator=(const output_memory_stream&) = delete;

    uint32_t size() const {
        return pos_;
    }

    const void* data() const {
        return data_;
    }

    uint32_t position() const {
        return pos_;
    }

    void position(uint32_t pos) {
        assert(pos <= cap_);
        pos_ = pos;
    }

    std::string str() const {
        return std::string{reinterpret_cast<char*>(data_), size()};
    }

    template<typename T>
    inline void write(const T& value) noexcept {
        constexpr uint32_t size = sizeof(T);
        ensure(size);
        details::copy(&value, size, data_ + pos_);
        pos_ += size;
    }

    inline void write(const void* data, const uint32_t size) noexcept {
        ensure(size);
        details::copy(data, size, data_ + pos_);
        pos_ += size;
    }

    inline void ensure(const uint32_t free_space) noexcept {
        if (pos_ + free_space >= cap_) {
            grow(pos_ + free_space);
        }
    }

    void grow(const uint32_t required_size) noexcept {
        auto capacity = cap_ << 1u;
        if (capacity < required_size) {
            capacity = required_size << 1u;
        }
        auto* buffer = new(std::nothrow) std::byte[capacity];
        details::copy(data_, pos_, buffer);
        delete[] data_;
        data_ = buffer;
        cap_ = capacity;
    }

private:
    std::byte* data_;
    uint32_t pos_;
    uint32_t cap_;
};


template<>
struct is_readable_stream_type<input_memory_stream> : public std::true_type {
};

template<>
struct is_readable_stream_type<output_memory_stream> : public std::false_type {
};


}