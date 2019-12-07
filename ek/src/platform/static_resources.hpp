#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <functional>

namespace ek {

std::vector<uint8_t> get_content(const char* path);

bool save_content(const char* path, const std::vector<uint8_t>& buffer);

std::vector<uint8_t> get_resource_content(const char* path);

inline std::vector<uint8_t> get_resource_content(const std::string& path) {
    return get_resource_content(path.c_str());
}

void get_resource_content_async(const char* path, std::function<void(std::vector<uint8_t>)> callback);

}


