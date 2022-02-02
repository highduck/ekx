#ifndef EKX_NG_UPDATER_H
#define EKX_NG_UPDATER_H

#include <ekx/app/time_layers.h>
#include <ecxx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct updater_state updater_state;

typedef void (* updater_callback_t)(updater_state* state);

struct updater_state {
    entity_t e;
    TimeLayer time_layer;
    updater_callback_t callback;
};

static struct {
    uint32_t num;
    updater_state data[64];
    uint32_t init_guard;
} updater;

void updater_init(void);

void updater_set(entity_t e, updater_callback_t callback);

void updater_update();

void updater_remove(entity_t e);

#ifdef __cplusplus
}
#endif

#endif // EKX_NG_UPDATER_H
