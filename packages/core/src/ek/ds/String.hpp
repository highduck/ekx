#pragma once

#include <cstdarg>
#include <cstring>
#include <functional>
#include <ek/buf.h>
#include <ek/hash.h>
#include <ek/string.h>
#include <ek/print.h>
#include <ek/assert.h>
#include <ek/core_dbg.h>

namespace ek {

// TODO: add `clear` method
// TODO; `resize` should not change capacity and just set \0 in case of smaller size
class String final {
public:
    char* _buffer;

    String() noexcept: _buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_STRING);
    }

    String(const char* cstr) noexcept: _buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_STRING);
        if (cstr && *cstr != '\0') {
            const auto sz = strlen(cstr) + 1;
            ek_buf_set_size((void**) &_buffer, 1, sz, sz);
            memcpy(_buffer, cstr, sz);
        }
    }

    String(const char* data, uint32_t size) noexcept: _buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_STRING);
        if (data && *data != '\0') {
            const auto sz = size + 1;
            ek_buf_set_size((void**) &_buffer, 1, sz, sz);
            memcpy(_buffer, data, size);
            _buffer[size] = '\0';
        }
    }

    String(const String& m) noexcept: _buffer{nullptr} {
        ek_core_dbg_inc(EK_CORE_DBG_STRING);
        arr_init_from((void**) &_buffer, 1, m._buffer, ek_buf_length(m._buffer));
    }

    String(String&& m) noexcept: _buffer{m._buffer} {
        ek_core_dbg_inc(EK_CORE_DBG_STRING);
        m._buffer = nullptr;
    }

    void assign(const char* str, uint32_t size) {
        arr_init_from((void**) &_buffer, 1, str, size + 1);
    }

    String& operator=(const char* str) {
        arr_init_from((void**) &_buffer, 1, str, strlen(str) + 1);
        return *this;
    }

    String& operator=(const String& obj) {
        if (this != &obj) {
            arr_assign((void**) &_buffer, 1, obj._buffer);
        }
        return *this;
    }

    String& operator=(String&& m) noexcept {
        ek_buf_reset((void**) &_buffer);
        _buffer = m._buffer;
        m._buffer = nullptr;
        return *this;
    }

    ~String() noexcept {
        ek_core_dbg_dec(EK_CORE_DBG_STRING);
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
        //return _buffer ? hash_murmur2_64(_buffer, ek_buf_length(_buffer), HASH_MURMUR2_64_DEFAULT_SEED) : 0;
        return _buffer ? hash_fnv64(_buffer, HASH_FNV64_INIT) : 0;
    }

    [[nodiscard]]
    constexpr uint32_t hash32() const noexcept {
        //return _buffer ? hash_murmur2_64(_buffer, ek_buf_length(_buffer), HASH_MURMUR2_64_DEFAULT_SEED) : 0;
        return _buffer ? hash_fnv32(_buffer, HASH_FNV32_INIT) : 0;
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
        va_list va;
        va_start(va, fmt);
        ek_vsnprintf(buf, 1024, fmt, va);
        va_end(va);
        return buf;
    }
};

}

template<>
struct std::hash<ek::String> {
    std::size_t operator()(const ek::String& s) const noexcept {
        if (sizeof(std::size_t) == 4) {
            return s.hash32();
        } else {
            return s.hash();
        }
    }
};
