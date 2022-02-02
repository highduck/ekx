

static void ek_legacy_core_stub() {}

#include "core_dbg.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG
int _core_dbg_stats[EK_CORE_DBG_MAX_COUNT];

void ek_core_dbg_inc(int m) {
    ++_core_dbg_stats[m];
}

void ek_core_dbg_dec(int m) {
    --_core_dbg_stats[m];
}

int ek_core_dbg_get(int m) {
    return _core_dbg_stats[m];
}
#endif

#ifdef __cplusplus
}
#endif

////#ifdef EK_ALLOCATION_TRACKER

//#include <new>
//#include <cstring>
//
//void* operator new(size_t sz) {
//    if (sz == 0) {
//        ++sz;
//    }
//
//    if (void* ptr = std::malloc(sz)) {
//        memset(ptr, 0xCC, sz);
//        return ptr;
//    }
//
//    abort();
//}
//
//void operator delete(void* ptr) noexcept {
//    std::free(ptr);
//}

////#endif // EK_ALLOCATION_TRACKER