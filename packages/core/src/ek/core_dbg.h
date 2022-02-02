#ifndef EK_CORE_DBG_H
#define EK_CORE_DBG_H

#include "ek/log.h"

#ifdef __cplusplus
extern "C" {
#endif

enum {
    EK_CORE_DBG_ARRAY = 0,
    EK_CORE_DBG_POD_ARRAY,
    EK_CORE_DBG_SIGNAL,
    EK_CORE_DBG_HASH,
    EK_CORE_DBG_STRING,

    EK_CORE_DBG_INTERACTIVE,
    EK_CORE_DBG_VD,

    EK_CORE_DBG_MAX_COUNT,
};

extern int _core_dbg_stats[EK_CORE_DBG_MAX_COUNT];

#ifndef NDEBUG
void ek_core_dbg_inc(int m);

void ek_core_dbg_dec(int m);

int ek_core_dbg_get(int m);

#else
#define ek_core_dbg_inc(x) ((void)(x))
#define ek_core_dbg_dec(x) ((void)(x))
#define ek_core_dbg_get(x) (0)
#endif

#ifdef __cplusplus
}
#endif

#endif // EK_CORE_DBG_H
