#include <ek/admob.h>
#include <ek/log.h>

// implementation
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

struct ek_admob ek_admob;

void ek_admob_set_callback(ek_admob_callback callback, void* userdata) {
    ek_admob.callback = callback;
    ek_admob.userdata = userdata;
}

static void ek_admob__post(ek_admob_event_type event) {
    ek_admob_callback cb = ek_admob.callback;
    if(cb) {
        cb(ek_admob.userdata, event);
    }
}

static void ek_admob__init(void) {
    log_debug("admob initialize");
    memset(&ek_admob, 0, sizeof(ek_admob));
}

void ek_admob_shutdown(void) {
    log_debug("admob shutdown");
}

#if defined(__ANDROID__)

#include "admob_android.c.h"

#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

#include "admob_ios.m.h"

#else

bool ek_admob_supported(void) {
    return false;
}

void ek_admob_init(ek_admob_config config) {
    ek_admob__init();
    ek_admob.config = config;
}

void ek_admob_show_banner(int flags) {
    (void) flags;
}

void ek_admob_show_rewarded_ad(void) {
}

// returns `false` if interstitial is not loaded
void ek_admob_show_interstitial_ad(void) {
}

#endif
