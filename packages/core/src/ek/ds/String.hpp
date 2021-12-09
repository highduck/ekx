#pragma once

#include <cstdarg>
#include <cstring>
#include <functional>
#include <ek/buf.h>
#include <ek/murmur.h>
#include <ek/string.h>
#include <ek/assert.h>

namespace ek {

// TODO: add `clear` method
// TODO; `resize` should not change capacity and just set \0 in case of smaller size
class String final {
public:
    // DynamicBuffer is just handle, not RAII object
    char* _buffer = nullptr;

    constexpr String() noexcept = default;

    String(const char* cstr) noexcept: _buffer{nullptr} {
        if (cstr && *cstr != '\0') {
            const auto sz = strlen(cstr) + 1;
            ek_buf_set_size((void**) &_buffer, 1, sz, sz);
            memcpy(_buffer, cstr, sz);
        }
    }

    String(const char* data, uint32_t size) noexcept: _buffer{nullptr} {
        if (data && *data != '\0') {
            const auto sz = size + 1;
            ek_buf_set_size((void**) &_buffer, 1, sz, sz);
            memcpy(_buffer, data, size);
            _buffer[size] = '\0';
        }
    }

    String(const String& other) noexcept: _buffer{nullptr} {
        _copyFrom(other._buffer, ek_buf_length(other._buffer));
    }

    void _copyFrom(const char* data, uint32_t sz) {
        if (ek_buf_capacity(_buffer) < sz) {
            ek_buf_set_capacity((void**) &_buffer, sz, 1);
        }
        if (_buffer) {
            ek_buf_header(_buffer)->length = sz;
            memcpy(_buffer, data, sz);
        }
    }

    void assign(const char* str, uint32_t size) {
        EK_ASSERT(_buffer != str);
        _copyFrom(str, size + 1);
    }

    String& operator=(const char* str) {
        EK_ASSERT(_buffer != str);
        _copyFrom(str, strlen(str) + 1);
        return *this;
    }

    String& operator=(const String& obj) {
        if (this != &obj) {
            _copyFrom(obj._buffer, ek_buf_length(obj._buffer));
        }
        return *this;
    }

    constexpr String(String&& other) noexcept {
        _buffer = other._buffer;
        other._buffer = nullptr;
    }

    String& operator=(String&& other) noexcept {
        ek_buf_reset((void**) &_buffer);

        _buffer = other._buffer;
        other._buffer = nullptr;
        return *this;
    }

    ~String() noexcept {
        ek_buf_reset((void**) &_buffer);
    }

    [[nodiscard]]
    uint32_t size() const {
        return _buffer ? (ek_buf_length(_buffer) - 1) : 0;
    }

    [[nodiscard]]
    uint32_t capacity() const {
        return _buffer ? (ek_buf_capacity(_buffer) - 1) : 0;
    }

    [[nodiscard]]
    bool empty() const {
        return size() == 0;
    }

    [[nodiscard]]
    constexpr char* data() const {
        return _buffer;
    }

    [[nodiscard]]
    const char* find(const char* cstr) const {
        if (!cstr || !_buffer) {
            return nullptr;
        }
        return strstr(_buffer, cstr);
    }

    String operator+(const String& other) const {
        const auto sz1 = size();
        const auto sz2 = other.size();
        const auto sz = sz1 + sz2 + 1;
        String s;
        ek_buf_set_size((void**) &s._buffer, 1, sz, sz);
        if (sz1) {
            memcpy(s._buffer, _buffer, sz1);
        }
        if (sz2) {
            memcpy(s._buffer + sz1, other._buffer, sz2 + 1);
        }
        return s;
    }

    bool operator==(const char* cstr) const {
        if (cstr == _buffer) {
            return true;
        }
        const auto sz = size();
        if ((!cstr || *cstr == '\0') && sz == 0) {
            return true;
        }
        return _buffer && cstr && strcmp(_buffer, cstr) == 0;
    }

    bool operator==(const String& str) const {
        return *this == str.c_str();
    }

    bool operator!=(const char* cstr) const {
        return !(*this == cstr);
    }

    bool operator!=(const String& str) const {
        return !(*this == str);
    }

    [[nodiscard]]
    constexpr const char* c_str() const {
        return _buffer ? _buffer : "";
    }

    [[nodiscard]]
    bool startsWith(const char* cstr) const {
        const char* str = c_str();
        return strstr(str, cstr) == str;
    }

    [[nodiscard]]
    constexpr uint64_t hash() const noexcept {
        return _buffer ? ek_murmur64(_buffer, ek_buf_length(_buffer), EK_MURMUR_DEFAULT_SEED) : 0;
    }

    void clear() {
        resize(0);
    }

    void reserve(uint32_t cap) {
        const auto sz = cap + 1;
        if (sz > ek_buf_capacity(_buffer)) {
            ek_buf_set_capacity((void**) &_buffer, sz, 1);
        }
    }

    void resize(uint32_t len) {
        reserve(len);
        if (_buffer) {
            ek_buf_header(_buffer)->length = len + 1;
            _buffer[len] = '\0';
        }
    }

    [[nodiscard]]
    char back() const {
        return _buffer ? _buffer[size() - 1] : '\0';
    }

    char pop_back() {
        char r = '\0';
        const auto sz = size();
        if (sz != 0) {
            r = c_str()[sz - 1];
            resize(sz - 1);
        }
        return r;
    }

    void push_back(char ch) {
        if (ek_buf_full(_buffer)) {
            ek_buf_set_capacity((void**) &_buffer, ek_buf_capacity(_buffer) + 1, 1);
        }
        const auto len = ek_buf_length(_buffer);
        // replace NULL to char
        data()[len - 1] = ch;
        // set NULL to the end
        data()[len] = '\0';
        ek_buf_header(_buffer)->length += 1;
    }

    String& operator+=(const String& other) {
        const auto sz1 = size();
        const auto sz2 = other.size();
        const auto sz = sz1 + sz2 + 1;
        ek_buf_set_size((void**) &_buffer, 1, sz, sz);
        if (sz2) {
            memcpy(_buffer + sz1, other._buffer, sz2 + 1);
        }
        return *this;
    }

    String& operator+=(char ch) {
        push_back(ch);
        return *this;
    }

    static String format(const char* fmt, ...) {
        char buf[1024];
//        int result;
        va_list va;
        va_start(va, fmt);

//        result =
        stbsp_vsnprintf(buf, 1024, fmt, va);
        va_end(va);

        return buf;
    }
};

}
//
//bool operator==(const ek::String& lhs, const ek::String& rhs) {
//    return lhs == rhs;
//}

template<>
struct std::hash<ek::String> {
    std::size_t operator()(const ek::String& s) const noexcept {
        return (std::size_t) s.hash();
    }
};

/** string pool **/
//#include "Hash.hpp"
//
//namespace ek {
//
//struct Str {
//    const char* cstr;
//};
//
//struct StrPool {
//    Hash<Str> pool;
//
//    const char* cstr(const char* p) {
//
//    }
//};
//
//}