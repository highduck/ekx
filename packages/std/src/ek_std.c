#include <ek/assert.c>
#include <ek/base64.c>
#include <ek/bitset.c>
#include <ek/buf.c>
#include <ek/log.c>
#include <ek/print.c>
#include <ek/string.c>
#include <ek/time.c>
#include <ek/utf8.c>
#include <ek/io.c>
#include <ek/rnd.c>
#include <ek/ids.c>
#include <ek/math.c>
#include <ek/rr.c>
#include <ek/hash/murmur.c>
#include <ek/hash/hsp.c>

#ifdef __EMSCRIPTEN__
#include <ek/sparse_array.c>
#else
#include <ek/sparse_array_vm.c>
#endif

// non-standard
#ifndef NDEBUG
// TODO:
//static void perf_penalty(long ms, const char* msg) {
//    log_warn("SLOWDOWN: %s", msg);
//    const long ns = ms * 1000000L;
//    nanosleep(&(struct timespec) {
//            .tv_sec = 0,
//            .tv_nsec = ns
//    }, NULL);
//}
#endif
