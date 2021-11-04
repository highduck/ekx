#pragma once

#include "StringUtil.hpp"

namespace ek {

bool ends_with(const std::string& str, const char end) {
    return !str.empty() && str[str.size() - 1] == end;
}

bool ends_with(const std::string& str, const std::string& end) {
    return end.size() <= str.size()
           && str.rfind(end) == str.size() - end.size();
}

bool starts_with(const std::string& str, const char start) {
    return !str.empty()
           && str[0] == start;
}

bool starts_with(const std::string& str, const std::string& start) {
    return str.find(start) == 0;
}

//Array<std::string> split(const std::string& str, const char separator) {
//    using size_type = std::string::size_type;
//
//    Array<std::string> result{};
//    size_type prev = 0;
//    size_type pos = 0;
//
//    while ((pos = str.find(separator, pos)) != std::string::npos) {
//        result.push_back(str.substr(prev, pos - prev));
//        prev = ++pos;
//    }
//
//    // Last word
//    result.push_back(str.substr(prev, pos - prev));
//
//    return result;
//}

//std::string join(const Array<std::string>& strings, const std::string& delimiter) {
//    std::string res{};
//    size_t i = 0;
//    for (const auto& str : strings) {
//        if (0 != i++) {
//            res += delimiter;
//        }
//        res += str;
//    }
//    return res;
//}

//std::string replace(const std::string& str, const std::string& search, const std::string& repl) {
//    std::string res{str};
//    size_t start_pos = 0;
//    while ((start_pos = res.find(search, start_pos)) != std::string::npos) {
//        res.replace(start_pos, search.size(), repl);
//        start_pos += repl.size();
//    }
//    return res;
//}

int replace(char* str, char search, char repl) {
    int replacements = 0;
    if(str) {
        while(*str != '\0') {
            if(*str == search) {
                *str = repl;
                ++replacements;
            }
            ++str;
        }
    }
    return replacements;
}

inline void append_itoa_fixed_pad_2(int v, std::string& out) {
    if (v < 10) {
        out += '0';
    }
    out += std::to_string(v);
}

std::string format_time_mm_ss(int seconds, TimeFormatFlags flags) {
    std::string result;
    int hours = seconds / 3600;
    int minutes = (seconds / 60) % 60;
    int secs = seconds % 60;
    if (hours > 0 || (flags & TimeFormat_KeepHours) != 0) {
        append_itoa_fixed_pad_2(hours, result);
        result += ':';
    }
    append_itoa_fixed_pad_2(minutes, result);
    result += ':';
    append_itoa_fixed_pad_2(secs, result);
    return result;
}

bool equals_ignore_case(const char* a, const char* b) {
    return a && b && strcasecmp(a, b) == 0;
}

}