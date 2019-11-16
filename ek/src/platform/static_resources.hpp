#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>
#include <ek/array_buffer.hpp>

namespace ek {

array_buffer get_content(const char* path);

bool save_content(const char* path, const array_buffer& buffer);

array_buffer get_resource_content(const char* path);

inline array_buffer get_resource_content(const std::string& path) {
    return get_resource_content(path.c_str());
}

void get_resource_content_async(const char* path, std::function<void(array_buffer)> callback);

}


