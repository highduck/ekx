#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

struct PopupManager {
    ecs::EntityApi entity;
    PodArray<ecs::EntityApi> active;
    ecs::EntityApi closingLast;
    float fade_progress = 0.0f;
    float fade_duration = 0.3f;
    float fade_alpha = 0.5f;
    ecs::EntityApi back;
    ecs::EntityApi layer;
};

void init_basic_popup(ecs::EntityApi e);

void open_popup(ecs::EntityApi e);

void close_popup(ecs::EntityApi e);

void clear_popups();

void close_all_popups();

void update_popup_managers(float dt);

uint32_t count_active_popups();

}

extern ek::PopupManager g_popup_manager;
void popup_manager_init();
void popup_manager_update();
