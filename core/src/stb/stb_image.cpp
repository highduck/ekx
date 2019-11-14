// disable stb_image for Web target
// we need built-in browser image decoding feature

#if !defined(__EMSCRIPTEN__)

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#endif