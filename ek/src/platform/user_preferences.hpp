#pragma once

#include <string>
#include <vector>
#include <cstdint>

namespace ek {

void set_user_preference(const std::string& key, int value);

int get_user_preference(const std::string& key, int default_value = 0);

void set_user_string(const std::string& key, const std::string& str);

std::string get_user_string(const std::string& key, const std::string& default_value = "");

void set_user_data(const std::string& key, const std::vector<uint8_t>& buffer);

std::vector<uint8_t> get_user_data(const std::string& key);

}


