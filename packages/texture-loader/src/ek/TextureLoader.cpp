#include "TextureLoader.hpp"

#if defined(__EMSCRIPTEN__)

#include "TextureLoader_Web.hpp"

#elif defined(__ANDROID__)

#include "TextureLoader_Android.hpp"

#else

#include "TextureLoader_Generic.hpp"

#endif