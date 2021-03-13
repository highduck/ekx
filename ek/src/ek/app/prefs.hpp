#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ek {

void set_user_preference(const char* key, int value);

int get_user_preference(const char* key, int default_value = 0);

void set_user_string(const char* key, const char* str);

std::string get_user_string(const char* key, const char* default_value = "");

void set_user_data(const char* key, const uint8_t* data, uint32_t size);

std::vector<uint8_t> get_user_data(const char* key);

}


