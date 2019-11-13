#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <ek/array_buffer.hpp>

namespace ek {

array_buffer get_content(const char* path);

bool save_content(const char* path, const array_buffer& buffer);

array_buffer get_resource_content(const char* path);

inline array_buffer get_resource_content(const std::string& path) {
    return get_resource_content(path.c_str());
}

}


