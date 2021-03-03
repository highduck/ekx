#include "prefs.hpp"
#include <ek/assert.hpp>
#include <ek/util/base64.hpp>

namespace ek {

void set_user_data(const std::string& key, const std::vector<uint8_t>& buffer) {
    EK_ASSERT(!key.empty());

    auto size = base64::getEncodedMaxSize(buffer.size());
    std::string encoded;
    encoded.resize(size);
    size = base64::encode(encoded.data(), buffer.data(), buffer.size());
    encoded.resize(size);

    set_user_string(key, encoded);
}

std::vector<uint8_t> get_user_data(const std::string& key) {
    const std::string str = get_user_string(key, "");

    std::vector<uint8_t> decoded{};
    auto size = base64::getDecodedMaxSize(str.size());
    decoded.resize(size);
    size = base64::decode(decoded.data(), str.c_str(), str.size());
    decoded.resize(size);
    return decoded;
}

}
