#pragma once

#include <ecx/ecx.hpp>
#include <ek/ds/FixedArray.hpp>

namespace ek {

struct PopupManager {
    entity_t entity;
    entity_t back;
    entity_t layer;
    entity_t closing_last;
    float fade_progress;
    float fade_duration;
    float fade_alpha;
    FixedArray<entity_t, 8> active;
};

void init_basic_popup(entity_t e);

void open_popup(entity_t e);

void close_popup(entity_t e);

void clear_popups();

void close_all_popups();

void update_popup_managers(float dt);

uint32_t count_active_popups();

}

extern ek::PopupManager g_popup_manager;
void popup_manager_init();
void update_popup_manager();
