#include <ek/local_storage.h>
#include <ek/app.h>
#include <ek/base64.h>
#include <ek/assert.h>

void ek_ls_set_data(const char* key, const void* data, uint32_t size) {
    EK_ASSERT(key != 0 && data != 0);
    EK_ASSERT(size <= 0xFFFF);

    uint32_t b64size = base64_encode_size(size);
    char* encoded = malloc(b64size + 1);
    b64size = base64_encode(encoded, b64size, data, size);
    // null-terminate our base64 buffer
    encoded[b64size] = 0;
    ek_ls_set_s(key, encoded);
}

int ek_ls_get_data(const char* key, void* buffer, uint32_t buffer_size) {
    EK_ASSERT(key != 0);

    const uint32_t encoded_buf_size = base64_encode_size(buffer_size);
    char encoded_buf[encoded_buf_size];

    const int encoded_size = ek_ls_get_s(key, encoded_buf, buffer_size);
    if(encoded_size > 0) {
        return (int)base64_decode(buffer, buffer_size, encoded_buf, encoded_size);
    }
    return 0;
}

void ek_ls_set64(const char* key, int64_t value) {
    EK_ASSERT(key != 0);
    ek_ls_set_data(key, &value, sizeof(int64_t));
}

bool ek_ls_get64(const char* key, int64_t* value) {
    EK_ASSERT(key != 0 && value != 0);
    int64_t v;
    const int n = ek_ls_get_data(key, &v, sizeof(int64_t));
    if (n != 8) {
        return false;
    }
    if (value) {
        *value = v;
    }
    return true;
}

#if defined(__EMSCRIPTEN__)
#include "local_storage_js.c.h"
#elif defined(__ANDROID__)
#include "local_storage_android.c.h"
#elif defined(__APPLE__)
// separated .m file source
#else
#include "local_storage_null.c.h"
#endif
