#ifndef EK_BUF_H
#define EK_BUF_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_buf_header_t {
    uint32_t pad0;
    uint32_t pad1;
    uint32_t capacity;
    uint32_t length;
} ek_buf_header_t;

ek_buf_header_t* ek_buf_header(void* ptr);

uint32_t ek_buf_capacity(void* ptr);

uint32_t ek_buf_length(void* ptr);

bool ek_buf_full(void* ptr);

bool ek_buf_empty(void* ptr);

void ek_buf_reset(void** ptr);

void ek_buf_set_capacity(void** ptr, uint32_t newCapacity, uint32_t elementSize);

void ek_buf_set_size(void** buf, uint32_t elementSize, uint32_t len, uint32_t cap);

void* ek_buf_add_(void* ptr, uint32_t elementSize);

void* ek_buf_remove_(void* ptr, uint32_t i, uint32_t elementSize);

void* ek_buf_pop_(void* ptr, uint32_t elementSize);

#ifdef __cplusplus
}
#endif

#endif // EK_BUF_H
