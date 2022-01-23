#ifndef AUPH_IMPL
#define AUPH_IMPL
#else
#error You should implement auph once
#endif

#include "auph_addon.c"

#if defined(__EMSCRIPTEN__)

#include "auph_web.c"

#else

#include "auph_native.c"
#include "native/buffer.c"

#endif
