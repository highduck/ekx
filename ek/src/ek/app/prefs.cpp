#include "prefs.hpp"
#include <ek/assert.hpp>
#include <ek/util/base64.hpp>

namespace ek {

void set_user_data(const char* key, const uint8_t* data, uint32_t size) {
    EK_ASSERT(key != nullptr && data != nullptr);

    auto encodedSize = base64::getEncodedMaxSize(size);
    std::string encoded;
    encoded.resize(encodedSize);
    encodedSize = base64::encode(encoded.data(), data, size);
    encoded.resize(encodedSize);

    set_user_string(key, encoded.c_str());
}

std::vector<uint8_t> get_user_data(const char* key) {
    EK_ASSERT(key != nullptr);

    const std::string str = get_user_string(key);

    std::vector<uint8_t> decoded{};
    auto decodedSize = base64::getDecodedMaxSize(str.size());
    decoded.resize(decodedSize);
    decodedSize = base64::decode(decoded.data(), str.c_str(), str.size());
    decoded.resize(decodedSize);
    return decoded;
}

}
