#ifndef AUPH_IMPL
#define AUPH_IMPL
#else
#error You should implement auph once
#endif

#ifndef AUPH_ASSERT
#include <assert.h>
#define AUPH_ASSERT(x) assert(x)
#endif

#ifndef AUPH_LOG
#include <stdio.h>
#define AUPH_LOG(x) puts(x "\n")
#endif

#include "auph_addon.c.h"

#if defined(__EMSCRIPTEN__)

#include "auph_web.c.h"

#else

#include "auph_native.c.h"
#include "native/buffer.c.h"

#endif
