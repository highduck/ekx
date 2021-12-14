#include <ek/texture_loader.h>
#include <ek/assert.h>
#include <string.h>
#include <stdlib.h>

void ek_texture_loader_set_path(ek_texture_loader_path* dst, const char* src) {
    EK_ASSERT(dst != 0);
    EK_ASSERT(src != 0);
    strncpy(dst->path, src, EK_TEXTURE_LOADER_PATH_MAX);
}

void ek_texture_loader_init(ek_texture_loader* loader) {
    EK_ASSERT(loader != 0);
    memset(loader, 0, sizeof(ek_texture_loader));
    loader->premultiplyAlpha = true;
    loader->formatMask = 1;
}

ek_texture_loader* ek_texture_loader_create(void) {
    ek_texture_loader* loader = (ek_texture_loader*) malloc(sizeof(ek_texture_loader));
    EK_ASSERT(loader != 0);
    ek_texture_loader_init(loader);
    return loader;
}

void ek_texture_loader_destroy(ek_texture_loader* loader) {
    EK_ASSERT(loader != 0);
    free(loader);
}

#if defined(__EMSCRIPTEN__)

#include "texture_loader_web.c.h"

#elif defined(__ANDROID__)

#include "texture_loader_android.c.h"

#else

#include "texture_loader_generic.c.h"

#endif