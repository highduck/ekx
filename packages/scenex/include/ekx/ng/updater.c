#include "updater.h"

void updater_init(void) {
    add_ecs_erase_callback(updater_remove);
}

void updater_set(entity_t e, updater_callback_t callback) {
    for (uint32_t i = 0; i < updater.num; ++i) {
        if (updater.data[i].e == e) {
            updater.data[i].callback = callback;
            return;
        }
    }
    EK_ASSERT(updater.num < 64);
    updater.data[updater.num++] = (updater_state) {e, 0, callback};
}

void updater_update() {
    for (uint32_t i = 0; i < updater.num; ++i) {
        updater_state s = updater.data[i];
        if (s.callback) {
            s.callback(&s);
        }
    }
}

void updater_remove(entity_t e) {
    for (uint32_t i = 0; i < updater.num; ++i) {
        if (updater.data[i].e == e) {
            --updater.num;
            if (i < updater.num) {
                updater.data[i] = updater.data[updater.num];
            }
            return;
        }
    }
}
