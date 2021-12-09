#include "LocalStorage.hpp"
#include <ek/app/app.hpp>
#include <ek/base64.h>

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

    auto encodedSize = base64_encode_size(size);
    Array<char> encoded;
    encoded.resize(encodedSize);
    encodedSize = base64_encode(encoded.data(), encodedSize, data, size);
    encoded.resize(encodedSize);

    set_user_string(key, encoded.data());
}

void get_user_data(const char* key, Array<uint8_t>& output) {
    EKAPP_ASSERT(key != nullptr);

    const String str = get_user_string(key);

    auto decodedSize = base64_decode_size(str.size());
    output.resize(decodedSize);
    decodedSize = base64_decode(output.data(), decodedSize, str.c_str(), str.size());
    output.resize(decodedSize);
}

namespace UserPreferences {

uint32_t read(const char* key, uint8_t* buffer, uint32_t bufferSize) {
    EKAPP_ASSERT(key != nullptr);

    const auto str = get_user_string(key);
    const auto inputData = str.data();
    const auto inputSize = str.size();

    auto decodedSize = base64_decode_size(inputSize);
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
