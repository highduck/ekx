#include "assert.c.h"
#include "base64.c.h"
#include "bitset.c.h"
#include "buf.c.h"
#include "log.c.h"
#include "murmur.c.h"
#include "rnd.c.h"
#include "string.c.h"
#include "time.c.h"
#include "utf8.c.h"

#include "image/fastblur.c.h"
#include "image/image.c.h"

/*
#ifdef __EMSCRIPTEN__
#include "walloc.c.h"

typedef __SIZE_TYPE__ size_t;

#define WASM_EXPORT(name) \
  __attribute__((export_name(#name))) \
  name

// Pull these in from walloc.c.
void *malloc(size_t size);
void free(void *p);

void* WASM_EXPORT(walloc)(size_t size) {
    return malloc(size);
}

void WASM_EXPORT(wfree)(void* ptr) {
    free(ptr);
}

#endif
 */
