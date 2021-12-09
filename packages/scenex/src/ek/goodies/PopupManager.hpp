#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

struct close_timeout {
    float time;
};

struct PopupManager {
    Array<ecs::EntityApi> active;
    ecs::EntityApi closingLast;
    float fade_progress = 0.0f;
    float fade_duration = 0.3f;
    float fade_alpha = 0.5f;
    ecs::EntityApi back;
    ecs::EntityApi layer;

    static ecs::EntityApi make();
    static void updateAll();

    static ecs::EntityApi Main;
};

ECX_TYPE(22, close_timeout);
ECX_TYPE(23, PopupManager);

void init_basic_popup(ecs::EntityApi e);

void open_popup(ecs::EntityApi e);

void close_popup(ecs::EntityApi e);

void clear_popups();

void close_all_popups();

void update_popup_managers(float dt);

uint32_t count_active_popups();

}