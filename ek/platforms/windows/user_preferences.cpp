#include <ek/util/base64.hpp>
#include <cassert>

namespace ek {

void set_user_preference(const std::string& key, int value) {
    assert(!key.empty());
}

int get_user_preference(const std::string& key, int default_value) {
    assert(!key.empty());
    return default_value;
}

void set_user_string(const std::string& key, const std::string& str) {
    assert(!key.empty());
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    assert(!key.empty());
    return default_value;
}

}