#pragma once

#include "../ds/String.hpp"
#include <ek/string.h>

namespace ek {

namespace Path {

inline void appendJoin(String& dest, const char* path) {
    char buf[1024];
    uint32_t size = ek_path_join(buf, 1024, dest.c_str(), path);
    dest.assign(buf, size);
//    if (path != nullptr) {
//        // skip all DELIMITER characters in start of `path`
//        while (ek_path_is_del(*path)) {
//            ++path;
//        }
//        if (!dest.empty()) {
//            const auto lastChar = dest.back();
//            if (!ek_path_is_del(lastChar)) {
//                dest.push_back('/');
//            }
//        }
//        dest += path;
//    }
}

inline String directory(const char* path) {
    char buf[1024];
    ek_path_dirname(buf, 1024, path);
    return String{buf};
}

}

inline String operator/(const String& p1, const char* p2) noexcept {
    String res{p1};
    Path::appendJoin(res, p2);
    return res;
}

inline String operator/(const String& p1, const String& p2) noexcept {
    return p1 / p2.c_str();
}

inline String& operator/=(String& p, const char* p2) noexcept {
    Path::appendJoin(p, p2);
    return p;
}

}


