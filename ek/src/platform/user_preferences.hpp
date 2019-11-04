#pragma once

#include <string>
#include <ek/array_buffer.hpp>

namespace ek {

void set_user_preference(const std::string& key, int value);

int get_user_preference(const std::string& key, int default_value = 0);

void set_user_string(const std::string& key, const std::string& str);

std::string get_user_string(const std::string& key, const std::string& default_value = "");

void set_user_data(const std::string& key, const array_buffer& buffer);

array_buffer get_user_data(const std::string& key);

}


