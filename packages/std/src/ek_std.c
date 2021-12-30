#include "std/assert.c.h"
#include "std/base64.c.h"
#include "std/bitset.c.h"
#include "std/buf.c.h"
#include "std/log.c.h"
#include "std/murmur.c.h"
#include "std/rnd.c.h"
#include "std/print.c.h"
#include "std/string.c.h"
#include "std/time.c.h"
#include "std/utf8.c.h"
#include "std/handle.c.h"

#define EK_HANDLE_2_IMPLEMENTATION
#include <ek/handle2.h>

#define EK_REF_IMPLEMENTATION
#include <ek/ref.h>

#ifdef __EMSCRIPTEN__
#include "std/sparse_array.c.h"
#else
#include "std/sparse_array_vm.c.h"
#endif

// non-standard
#ifndef NDEBUG
// TODO:
static void ek_slowdown_(const char* msg) {
    EK_WARN("SLOWDOWN: %s", msg);
    nanosleep(&(struct timespec) {
            .tv_sec = 0,
            .tv_nsec = 16 * 1000000
    }, NULL);
}
#endif


