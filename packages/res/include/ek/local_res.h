#ifndef EK_LOCAL_RES_H
#define EK_LOCAL_RES_H

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

struct ek_local_res;

typedef void (* ek_local_res_callback)(struct ek_local_res* lr);

typedef struct ek_local_res {
    uint8_t* buffer;
    size_t length;
    void* userdata;
    int32_t status;
    void* handle;
    ek_local_res_callback closeFunc;
} ek_local_res;

bool ek_local_res_success(const ek_local_res* lr);
void ek_local_res_close(ek_local_res* lr);

void ek_local_res_load(const char* path, ek_local_res_callback callback, void* userdata);

#ifndef __EMSCRIPTEN__

int ek_local_res_get_file_platform(const char* path, ek_local_res* lr);

#endif // not __EMSCRIPTEN__

#ifdef __cplusplus
}
#endif

#endif // EK_LOCAL_RES_H
