#include "updater.h"

ecx_component_type updater_comp_type;

void updater_init(void) {
    init_component_type(&updater_comp_type, (ecx_component_type_decl) {
            "updater", 8, 1, {sizeof(updater_state)}
    });
}

void updater_set(entity_t e, updater_callback_t callback) {
    EK_ASSERT(updater_comp_type.index);
    updater_state* state = add_component(&updater_comp_type, e);
    state->callback = callback;
    state->time_layer = 0;
}

void updater_update() {
    for (uint32_t i = 1; i < updater_comp_type.size; ++i) {
        entity_idx_t ei = updater_comp_type.handleToEntity[i];
        updater_state s = ((updater_state*) updater_comp_type.data[0])[i];
        if (s.callback) {
            s.callback(entity_at(ei), s.time_layer);
        }
    }
}

void updater_remove(entity_t e) {
    remove_component(&updater_comp_type, e);
}
