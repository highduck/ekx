#ifndef EK_BASE64_H
#define EK_BASE64_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t base64_encode_size(uint32_t length);

uint32_t base64_decode_size(uint32_t length);

uint32_t base64_encode(void* dst, uint32_t dstMaxSize, const void* src, uint32_t srcSize);

uint32_t base64_decode(void* dst, uint32_t dstMaxSize, const void* src, uint32_t srcSize);

#ifdef __cplusplus
}
#endif

#endif //EK_BASE64_H
