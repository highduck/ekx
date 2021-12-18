#ifndef EK_ADMOB_H
#define EK_ADMOB_H

#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ek_admob_event_type {
    EK_ADMOB_INITIALIZED = 0,
    EK_ADMOB_VIDEO_REWARDED = 1,
    EK_ADMOB_VIDEO_LOADED = 2,
    EK_ADMOB_VIDEO_FAILED = 3,
    EK_ADMOB_VIDEO_LOADING = 4,
    EK_ADMOB_VIDEO_CLOSED = 5,
    EK_ADMOB_INTERSTITIAL_CLOSED = 6,
} ek_admob_event_type;

typedef enum ek_admob_child_directed {
    EK_ADMOB_CHILD_DIRECTED_UNSPECIFIED = -1,
    EK_ADMOB_CHILD_DIRECTED_FALSE = 0,
    EK_ADMOB_CHILD_DIRECTED_TRUE = 1
} ek_admob_child_directed;

typedef struct ek_admob_config {
    const char* banner;
    const char* video;
    const char* inters;
    ek_admob_child_directed child_directed;
} ek_admob_config;

typedef void (* ek_admob_callback)(void* userdata, ek_admob_event_type event);

struct ek_admob {
    ek_admob_config config;
    ek_admob_callback callback;
    void* userdata;
};

extern struct ek_admob ek_admob;

void ek_admob_set_callback(ek_admob_callback callback, void* userdata);

bool ek_admob_supported(void);

void ek_admob_init(ek_admob_config config);

void ek_admob_show_banner(int flags);

void ek_admob_show_rewarded_ad(void);

void ek_admob_show_interstitial_ad(void);

#ifdef __cplusplus
}
#endif

#endif // EK_ADMOB_H
