#ifndef EKX_NG_UPDATER_H
#define EKX_NG_UPDATER_H

#include <ekx/app/time_layers.h>
#include <ecx/ecx.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct updater_state updater_state;

typedef void (* updater_callback_t)(entity_t e, TimeLayer time_layer);

struct updater_state {
    TimeLayer time_layer;
    updater_callback_t callback;
};

void updater_init(void);

void updater_set(entity_t e, updater_callback_t callback);

void updater_update();

void updater_remove(entity_t e);

#ifdef __cplusplus
}
#endif

#endif // EKX_NG_UPDATER_H
