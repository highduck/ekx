#ifndef EK_PRE_H
#define EK_PRE_H

#include <ek/config.h>

#ifdef __cplusplus
#include <cstdint>
#include <cstdbool>
#include <csignal>
#include <cstring>
#include <cstdlib>
#else
#include <stdint.h>
#include <stdbool.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#endif

#define EK_FALLTHROUGH __attribute__((fallthrough));

#ifdef EK_SOURCE_LOCATION_ENABLED
#define EK_CURRENT_FILE __FILE__
#define EK_CURRENT_LINE __LINE__
#else // EK_SOURCE_LOCATION_ENABLED
#define EK_CURRENT_FILE ""
#define EK_CURRENT_LINE 0
#endif // !EK_SOURCE_LOCATION_ENABLED

// require all literals 64-bit aligned
#define EK_STR_ALIGN_64(S) ((const union { char s[((sizeof (S) / 8) + 1) * 8]; uint64_t align; }){ S }.s)
//#define EK_STR_ALIGN_64(S) ((const struct { char s[((sizeof (S) / 8) + 1) * 8]; }){ S }.s)
//#define EK_STR_ALIGN_64(S) ((const char s[((sizeof (S) / 8) + 1) * 8]){S})
#define EK_UTF8_NEW(UTF8) EK_STR_ALIGN_64(u8##UTF8)

typedef uint8_t ek_u8;
typedef uint16_t ek_u16;
typedef uint32_t ek_u32;
typedef uint64_t ek_u64;
typedef int8_t ek_i8;
typedef int16_t ek_i16;
typedef int32_t ek_i32;
typedef int64_t ek_i64;
typedef float ek_f32;
typedef double ek_f64;

#define EK_PRAGMA(x) _Pragma(#x)

#define EK_DISABLE_WARNING_BEGIN() EK_PRAGMA(clang diagnostic push)
#define EK_DISABLE_WARNING(option) EK_PRAGMA(clang diagnostic ignored #option)
#define EK_DISABLE_WARNING_END() EK_PRAGMA(clang diagnostic pop)

#endif // EK_PRE_H
