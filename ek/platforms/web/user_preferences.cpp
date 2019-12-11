#include <ek/util/base64.hpp>
#include <cassert>
#include <emscripten.h>

namespace ek {

void set_user_preference(const std::string& key, int value) {
    EM_ASM({
               window.localStorage.setItem(UTF8ToString($0), $1);
           }, key.c_str(), value);
}

int get_user_preference(const std::string& key, int default_value) {
    return EM_ASM_INT({
                          var item = window.localStorage.getItem(UTF8ToString($0));
                          if (item != null) {
                              var val = parseInt(item);
                              if (val != null) {
                                  return val;
                              }
                          }
                          return $1;
                      }, key.c_str(), default_value);
}

void set_user_string(const std::string& key, const std::string& value) {
    EM_ASM({
               window.localStorage.setItem(UTF8ToString($0), UTF8ToString($1));
           }, key.c_str(), value.c_str());
}

std::string get_user_string(const std::string& key, const std::string& default_value) {
    char* buffer = (char*) EM_ASM_INT({
                                          var item = window.localStorage.getItem(UTF8ToString($0));
                                          if (item != null) {
                                              var lengthBytes = lengthBytesUTF8(item) + 1;
                                              var stringOnWasmHeap = _malloc(lengthBytes);
                                              return stringToUTF8(item, stringOnWasmHeap, lengthBytes);
                                          }
                                          return 0;
                                      }, key.c_str());
    if (buffer) {
        std::string str{buffer};
        free(buffer);
        return str;
    }
    return default_value;
}

void set_user_data(const std::string& key, const std::vector<uint8_t>& buffer) {
    assert(!key.empty());
    set_user_string(key, ek::base64::encode(buffer));
}

std::vector<uint8_t> get_user_data(const std::string& key) {
    return ek::base64::decode(get_user_string(key, ""));
}

}