#pragma once

#include "core.hpp"

#include <ek/assert.h>
#include <cstdint>
#include <cstdlib>
#include <cstring>

namespace ek {

class input_memory_stream {
public:

    input_memory_stream() = delete;

    input_memory_stream(const void* data, size_t size) :
            data_{static_cast<const uint8_t*>(data)},
            size_{static_cast<uint32_t>(size)},
            pos_{0u} {
        EK_ASSERT(size < 0x1FFFFFFF);
    }

    input_memory_stream(const input_memory_stream&) = delete;

    input_memory_stream(input_memory_stream&&) = delete;

    input_memory_stream& operator=(input_memory_stream&&) = delete;

    input_memory_stream& operator=(const input_memory_stream&) = delete;

    [[nodiscard]]
    inline uint32_t position() const {
        return pos_;
    }

    void position(uint32_t value) {
        EK_ASSERT(value <= size_);
        pos_ = value;
    }

    [[maybe_unused]]
    [[nodiscard]]
    inline bool eos() const {
        return pos_ >= size_;
    }

    uint32_t seek(const int32_t offset) {
        EK_ASSERT(pos_ + offset <= size_);
        return pos_ += offset;
    }

    inline void read(void* dest, const uint32_t size) noexcept {
        EK_ASSERT(pos_ + size <= size_);
        EK_ASSERT(pos_ + size >= pos_); // check overflow
        memcpy(dest, data_ + pos_, size);
        pos_ += size;
    }

    template<typename T>
    inline void read(T& to_value) noexcept {
        constexpr uint32_t size = sizeof(T);
        EK_ASSERT(pos_ + size <= size_);
        memcpy(&to_value, data_ + pos_, size);
        pos_ += size;
    }

    template<typename T>
    inline T read() noexcept {
        T value;
        read(value);
        return value;
    }

    [[nodiscard]]
    inline uint32_t size() const {
        return size_;
    }

    [[nodiscard]]
    inline const void* data() const {
        return data_;
    }

    [[nodiscard]]
    inline const void* dataAtPosition() const {
        return data_ + pos_;
    }

private:
    const uint8_t* data_;
    const uint32_t size_;
    uint32_t pos_;
};

class output_memory_stream {
public:

    output_memory_stream() = delete;

    explicit output_memory_stream(size_t capacity) :
            data_{(uint8_t*) malloc(capacity)},
            pos_{0u},
            cap_{(uint32_t)capacity} {
        EK_ASSERT(capacity < 0x7FFFFFFF);
    }

    ~output_memory_stream() {
        free(data_);
    }

    output_memory_stream(const output_memory_stream&) = delete;

    output_memory_stream(output_memory_stream&&) = delete;

    output_memory_stream& operator=(output_memory_stream&&) = delete;

    output_memory_stream& operator=(const output_memory_stream&) = delete;

    [[nodiscard]]
    inline uint32_t size() const {
        return pos_;
    }

    [[nodiscard]]
    inline const void* data() const {
        return data_;
    }

    [[nodiscard]]
    inline uint32_t position() const {
        return pos_;
    }

    inline void position(uint32_t pos) {
        EK_ASSERT(pos <= cap_);
        pos_ = pos;
    }

    template<typename T>
    inline void write(const T& value) noexcept {
        constexpr uint32_t size = sizeof(T);
        ensure(size);
        memcpy(data_ + pos_, &value, size);
        pos_ += size;
    }

    inline void write(const void* data, const uint32_t size) noexcept {
        ensure(size);
        memcpy(data_ + pos_, data, size);
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
        auto* buffer = (uint8_t*) malloc(capacity);
        memcpy(buffer, data_, pos_);
        free(data_);
        data_ = buffer;
        cap_ = capacity;
    }

private:
    uint8_t* data_;
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