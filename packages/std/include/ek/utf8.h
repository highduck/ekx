#ifndef EK_UTF8_H
#define EK_UTF8_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

//uint32_t ek_utf8_decode(uint32_t* state, uint32_t* codepoint, uint32_t byte);

uint32_t ek_utf8_next(const char** iter);

#ifdef __cplusplus
}
#endif

#endif //EK_UTF8_H
