#ifndef EK_PRINT_H
#define EK_PRINT_H

//#ifdef __EMSCRIPTEN__

#include <stb/stb_sprintf.h>

#define ek_snprintf(buf, count, fmt, ...) stbsp_snprintf(buf, count, fmt, __VA_ARGS__)
#define ek_vsnprintf(buf, count, fmt, va) stbsp_vsnprintf(buf, count, fmt, va)

//#else
//
//#include <stdio.h>
//
//#define ek_snprintf(buf, count, fmt, ...) snprintf(buf, count, fmt, __VA_ARGS__)
//
//#endif

#ifdef __cplusplus
extern "C" {
#endif




#ifdef __cplusplus
}
#endif

#endif // EK_PRINT_H
