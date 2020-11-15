#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

struct close_timeout {
    float time;
};

struct popup_manager_t {
    std::vector<ecs::entity> active;
    float fade_progress = 0.0f;
    float fade_duration = 0.4f;
    float fade_alpha = 0.5f;
    ecs::entity back;
    ecs::entity layer;
};

void init_basic_popup(ecs::entity e);

void open_popup(ecs::entity e);

void close_popup(ecs::entity e);

void clear_popups();

void close_all_popups();

ecs::entity create_popup_manager();

void update_popup_managers(float dt);

uint32_t count_active_popups();

ecs::entity get_popup_manager();

}