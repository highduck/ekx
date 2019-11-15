#pragma once

#include <cstring>
#include <string>
#include <vector>
#include <cmath>

#if defined(_WIN32)
#define strcasecmp stricmp
#endif

namespace ek {

bool ends_with(const std::string& str, char end);

bool ends_with(const std::string& str, const std::string& end);

bool starts_with(const std::string& str, char start);

bool starts_with(const std::string& str, const std::string& start);

std::vector<std::string> split(const std::string& str, char separator);

std::string join(const std::vector<std::string>& strings, const std::string& delimiter);

std::string replace(const std::string& str, const std::string& search, const std::string& replace);

namespace cstr {

inline bool equals(const char* a, const char* b) {
    return a && strcmp(a, b) == 0;
}

inline bool equals(const std::string& a, const char* b) {
    return b && strcmp(a.c_str(), b) == 0;
}

inline bool equals_ignore_case(const char* a, const char* b) {
    return a && b && strcasecmp(a, b) == 0;
}

inline bool equals_ignore_case(const std::string& a, const char* b) {
    return b && strcasecmp(a.c_str(), b) == 0;
}

}

// Game utilities
std::string format_time_mm_ss(int seconds);

inline std::string format_time_mm_ss(float seconds) {
    return format_time_mm_ss(static_cast<int>(std::ceil(seconds)));
}

// conversion
template<typename Integer>
inline std::string to_hex(Integer w, size_t hex_len = sizeof(Integer) << 1u) {
    static const char* digits = "0123456789ABCDEF";
    std::string rc(hex_len, '0');
    for (size_t i = 0, j = (hex_len - 1) * 4; i < hex_len; ++i, j -= 4) {
        rc[i] = digits[(w >> j) & 0x0F];
    }
    return rc;
}

}