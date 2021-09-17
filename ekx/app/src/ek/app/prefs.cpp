#include "prefs.hpp"
#include <ek/app/app.hpp>
#include <ek/util/Base64.hpp>

namespace ek {

void set_user_data(const char* key, const uint8_t* data, uint32_t size) {
    EKAPP_ASSERT(key != nullptr && data != nullptr);

    auto encodedSize = base64::getEncodedMaxSize(size);
    std::string encoded;
    encoded.resize(encodedSize);
    encodedSize = base64::encode(encoded.data(), data, size);
    encoded.resize(encodedSize);

    set_user_string(key, encoded.c_str());
}

std::vector<uint8_t> get_user_data(const char* key) {
    EKAPP_ASSERT(key != nullptr);

    const std::string str = get_user_string(key);

    std::vector<uint8_t> decoded{};
    auto decodedSize = base64::getDecodedMaxSize(str.size());
    decoded.resize(decodedSize);
    decodedSize = base64::decode(decoded.data(), str.c_str(), str.size());
    decoded.resize(decodedSize);
    return decoded;
}

namespace UserPreferences {

uint32_t read(const char* key, uint8_t* buffer, uint32_t bufferSize) {
    EKAPP_ASSERT(key != nullptr);

    const auto str = get_user_string(key);
    const auto inputData = str.data();
    const auto inputSize = str.size();

    auto decodedSize = base64::getDecodedMaxSize(inputSize);
    if (buffer && decodedSize <= bufferSize) {
        return base64::decode(buffer, inputData, inputSize);
    }
    return 0;
}

void set(const char* key, int64_t value) {
    EKAPP_ASSERT(key != nullptr);
    union {
        int64_t value;
        uint8_t bytes[8];
    } bc{value};
    set_user_data(key, bc.bytes, 8);
}

bool get(const char* key, int64_t* value) {
    EKAPP_ASSERT(key != nullptr);
    union {
        int64_t value;
        uint8_t bytes[8];
    } bc{};
    const auto n = read(key, bc.bytes, 8);
    if (n != 8) {
        return false;
    }
    if (value) {
        *value = bc.value;
    }
    return true;
}

}

}
