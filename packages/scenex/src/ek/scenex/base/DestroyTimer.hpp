#pragma once

#include <ecx/ecx.hpp>
#include <ekx/app/time_layers.h>
#include <ek/ds/PodArray.hpp>

typedef struct destroy_timer_t {
    entity_t entity;
    float delay;
    TimeLayer time_layer;
} destroy_timer_t;

typedef struct destroy_manager_t {
    ek::PodArray<destroy_timer_t> timers;
} destroy_manager_t;

void destroy_later(entity_t e, float delay = 0.0f, TimeLayer timer = {});

void destroy_children_later(entity_t e, float delay = 0.0f, TimeLayer timer = {});

extern destroy_manager_t g_destroy_manager;

void update_destroy_queue();
