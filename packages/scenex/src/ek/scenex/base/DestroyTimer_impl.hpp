#include "DestroyTimer.hpp"

#include <ek/scenex/base/Node.hpp>
#include <ek/time.h>
#include <ek/ds/Array.hpp>

destroy_manager_t g_destroy_manager;

void destroy_later(entity_t e, float delay, TimeLayer timer) {
    if(is_entity(e)) {
        destroy_timer_t t;
        t.entity = e;
        t.delay = delay;
        t.time_layer = timer;
        g_destroy_manager.timers.push_back(t);
        //e.reassign<DestroyTimer>(delay, timer);
    }
}

void destroy_children_later(entity_t e, float delay, TimeLayer timer) {
    auto it = ek::get_first_child(e);
    while (it.id) {
        //it.reassign<DestroyTimer>(delay, timer);
        destroy_later(it, delay, timer);
        it = ek::get_next_child(it);
    }
}

void update_destroy_queue() {
    using namespace ek;
    uint32_t i = 0;
    uint32_t end = g_destroy_manager.timers.size();
    while(i < end) {
        destroy_timer_t* timer = &g_destroy_manager.timers[i];
        entity_t e = timer->entity;
        if(is_entity(e)) {
            if (timer->delay > 0.0f) {
                timer->delay -= g_time_layers[timer->time_layer].dt;
                ++i;
                continue;
            }

            if (ecs::has<Node>(e)) {
                destroy_node(e);
            } else {
                destroy_entity(e);
            }
        }

        --end;
        if(i < end) {
            g_destroy_manager.timers[i] = g_destroy_manager.timers[end];
        }
    }
    if(end < g_destroy_manager.timers.size()) {
        g_destroy_manager.timers.resize(end);
    }
//        c.delay -= g_time_layers[c.timer].dt;
//        if (c.delay <= 0.0f && num != 4096) {
//            destroy_queue.push_back(e);
//            ++num;
//        }
//    }
//    for (auto e: destroy_queue) {
//        if (e.is_alive()) {
//            if (ecs::has<Node>(e)) {
//                destroy_node(e);
//            } else {
//                ecs::destroy(e);
//            }
//        }
//    }
}
