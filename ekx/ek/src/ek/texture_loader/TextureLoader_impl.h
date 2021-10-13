#pragma once

#include "TextureLoader.h"

#if defined(__EMSCRIPTEN__)

#include "TextureLoader_web.h"

#elif defined(__ANDROID__)

#include "TextureLoader_android.h"

#else

#include "TextureLoader_generic.h"

#endif