#ifndef STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION

#include <ek/log.h>
#include <ek/assert.h>

// enable NEON for all ARM targets on Android and iOS
#if defined(__aarch64__) || defined(__arm__)
#if defined(__APPLE__) || defined(__ANDROID__)
#define STBI_NEON
#endif // apple || android
#endif // arm

#define STBI_NO_STDIO
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_LINEAR
#define STBI_NO_HDR

#ifndef NDEBUG
#define STBI_NO_FAILURE_STRINGS
#else // DEBUG
#define STBI_FAILURE_USERMSG
#endif // NDEBUG

#define STBI_ASSERT(e)   EK_ASSERT(e)

#include <stb/stb_image.h>

#endif // STB_IMAGE_IMPLEMENTATION
