#pragma once

#include "Path.hpp"

namespace ek {

std::string path_join(const std::string& p1, const std::string& p2) {
    if (p1.empty()) {
        return p2;
    }

    if (p2.empty()) {
        return p1;
    }

    const char sep = '/';
    if (p1[p1.size() - 1] != sep) {
        return p1 + sep + p2;
    }
    return p1 + p2;
}

void path_extract_dir(char* dst, size_t maxSize, const char* path) {
    const char* name = path_name(path);
    unsigned len = 0;
    if (name && name != path) {
        len = name - path - 1;
        if (len > maxSize) {
            len = maxSize - 1;
        }
        strncpy(dst, path, len);
    }
    dst[len] = '\0';
}

const char* path_ext(const char* path) {
    if (path == nullptr) {
        return nullptr;
    }

    const char* extension = path;
    const char* last = nullptr;

    // find the last DOT
    while (*extension != '\0') {
        if (*extension == '.') {
            ++extension;
            last = extension;
        }
        ++extension;
    }

    return last ? last : extension;
}

const char* path_name(const char* path) {
    if (path == nullptr) {
        return nullptr;
    }

    const char* it = path;
    const char* last = path;

    // find the last DELIMITER
    while (*it != '\0') {
        if (isPathDelimiter(*it)) {
            ++it;
            last = it;
        }
        ++it;
    }

    return last;
}

std::string path_dir(const std::string& path) {
    const auto i = path.find_last_of('/');
    if (i != std::string::npos) {
        return path.substr(0, i);
    }
    return "";
}

}