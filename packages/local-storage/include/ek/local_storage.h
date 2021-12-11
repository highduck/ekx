#ifndef EK_LOCAL_STORAGE_H
#define EK_LOCAL_STORAGE_H

#include <ek/buf.h>

#ifdef __cplusplus
extern "C" {
#endif

void ek_ls_set_i(const char* key, int value);

int ek_ls_get_i(const char* key, int de_fault);

void ek_ls_set_s(const char* key, const char* str);

int ek_ls_get_s(const char* key, char* buffer, uint32_t buffer_size);

void ek_ls_set_data(const char* key, const void* data, uint32_t size);

int ek_ls_get_data(const char* key, void* buffer, uint32_t buffer_size);

void ek_ls_set64(const char* key, int64_t value);

bool ek_ls_get64(const char* key, int64_t* value);

#ifdef __cplusplus
}
#endif

#endif // EK_LOCAL_STORAGE_H
