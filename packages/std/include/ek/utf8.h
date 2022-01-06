#ifndef EK_UTF8_H
#define EK_UTF8_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t codepoint_t;

codepoint_t utf8_next(const char** iter);

#ifdef __cplusplus
}
#endif

#endif //EK_UTF8_H
