#pragma once

#include <ecxx/ecxx.hpp>
#include <ekx/app/time_layers.h>

typedef struct destroy_timer_t {
    entity_passport_t passport;
    float delay;
    TimeLayer time_layer;
} destroy_timer_t;

typedef struct destroy_manager_t {
    ek::PodArray<destroy_timer_t> timers;
} destroy_manager_t;

namespace ek {

//struct DestroyTimer {
//    float delay = 0.0f;
//    TimeLayer timer{};
//
//    static void updateAll();
//};

void destroyDelay(ecs::EntityApi e, float delay = 0.0f, TimeLayer timer = {});

void destroyChildrenDelay(ecs::EntityApi e, float delay = 0.0f, TimeLayer timer = {});

}

extern destroy_manager_t g_destroy_manager;

void destroy_manager_update();
