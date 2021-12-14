#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <ek/log.h>
#include <ek/assert.h>

// enable NEON for all ARM targets on Android and iOS
#if defined(__aarch64__) || defined(__arm__)
#if defined(__APPLE__) || defined(__ANDROID__)
#define STBI_NEON
#endif
#endif // arm

#ifndef EK_TOOLS
#define STBI_NO_STDIO
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#endif

#ifndef NDEBUG
#define STBI_NO_FAILURE_STRINGS
#else // DEBUG
#define STBI_FAILURE_USERMSG
#endif // NDEBUG

#define STBI_ASSERT(e)   EK_ASSERT(e)

#pragma clang diagnostic push
//#pragma clang diagnostic ignored "-Wsign-compare"
//#pragma clang diagnostic ignored "-Wmissing-field-initializers"
#pragma clang diagnostic ignored "-Wno-cast-align"

#include <stb/stb_image.h>

#pragma clang diagnostic pop

#endif // STB_IMAGE_IMPLEMENTATION