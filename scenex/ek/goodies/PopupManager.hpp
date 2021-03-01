#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

struct close_timeout {
    float time;
};

struct PopupManager {
    Array<ecs::entity> active;
    ecs::entity closingLast;
    float fade_progress = 0.0f;
    float fade_duration = 0.3f;
    float fade_alpha = 0.5f;
    ecs::entity back;
    ecs::entity layer;

    static ecs::entity make();
    static void updateAll();

    static ecs::entity Main;
};

void init_basic_popup(ecs::entity e);

void open_popup(ecs::entity e);

void close_popup(ecs::entity e);

void clear_popups();

void close_all_popups();

void update_popup_managers(float dt);

uint32_t count_active_popups();

}