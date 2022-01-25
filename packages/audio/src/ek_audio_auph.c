#include <auph/auph.h>

#include <ek/log.h>
#include <ek/assert.h>

#if 0
// for refactoring enable other formats
#define AUPH_WAV
#define AUPH_OGG
#define AUPH_FORCE_NATIVE_NULL_DEVICE
#endif

#define AUPH_MP3
#define AUPH_ASSERT(x) EK_ASSERT(x)
#define AUPH_LOG(x) log_debug(x)
#define AUPH_SETUP_EK_APP

#include <auph/auph.c>
