#include <ek/local_res.h>

bool ek_local_res_success(const ek_local_res* lr) {
    return lr->status == 0;
}

void ek_local_res_close(ek_local_res* lr) {
    if (lr->closeFunc) {
        lr->closeFunc(lr);
    }
}

#if defined(__ANDROID__)

#include "local_res_android.c.h"

#elif defined(__EMSCRIPTEN__)

// local_res_web.cpp

#else

#include "local_res_sys.c.h"

#endif
