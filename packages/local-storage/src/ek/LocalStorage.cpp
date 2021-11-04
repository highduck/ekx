#include "LocalStorage.hpp"
#include <ek/app/app.hpp>
#include <ek/util/Base64.hpp>

#if defined(__EMSCRIPTEN__)
#include "LocalStorage_Web.hpp"
#elif defined(__APPLE__)
#include "LocalStorage_Apple.hpp"
#elif defined(__ANDROID__)
#include "LocalStorage_Android.hpp"
#else
#include "LocalStorage_Null.hpp"
#endif

namespace ek {

void set_user_data(const char* key, const uint8_t* data, uint32_t size) {
    EKAPP_ASSERT(key != nullptr && data != nullptr);

    auto encodedSize = base64_encodedMaxSize(size);
    std::string encoded;
    encoded.resize(encodedSize);
    encodedSize = base64_encode(encoded.data(), encodedSize, data, size);
    encoded.resize(encodedSize);

    set_user_string(key, encoded.c_str());
}

std::vector<uint8_t> get_user_data(const char* key) {
    EKAPP_ASSERT(key != nullptr);

    const std::string str = get_user_string(key);

    std::vector<uint8_t> decoded{};
    auto decodedSize = base64_decodedMaxSize(str.size());
    decoded.resize(decodedSize);
    decodedSize = base64_decode(decoded.data(), decodedSize, str.c_str(), str.size());
    decoded.resize(decodedSize);
    return decoded;
}

namespace UserPreferences {

uint32_t read(const char* key, uint8_t* buffer, uint32_t bufferSize) {
    EKAPP_ASSERT(key != nullptr);

    const auto str = get_user_string(key);
    const auto inputData = str.data();
    const auto inputSize = str.size();

    auto decodedSize = base64_decodedMaxSize(inputSize);
    if (buffer && decodedSize <= bufferSize) {
        return base64_decode(buffer, decodedSize, inputData, inputSize);
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
