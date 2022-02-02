#include "DestroyTimer.hpp"

#include <ek/scenex/base/Node.hpp>
#include <ek/time.h>
#include <ek/ds/Array.hpp>

destroy_manager_t g_destroy_manager;

namespace ek {

void destroyDelay(ecs::EntityApi e, float delay, TimeLayer timer) {
    destroy_timer_t t;
    t.passport = ecs::EntityRef{e}.passport;
    t.delay = delay;
    t.time_layer = timer;
    g_destroy_manager.timers.push_back(t);
    //e.reassign<DestroyTimer>(delay, timer);
}

void destroyChildrenDelay(ecs::EntityApi e, float delay, TimeLayer timer) {
    auto it = e.get<Node>().child_first;
    while (it) {
        //it.reassign<DestroyTimer>(delay, timer);
        destroyDelay(it, delay, timer);
        it = it.get<Node>().sibling_next;
    }
}
}


void destroy_manager_update() {
    using namespace ek;
    uint32_t i = 0;
    uint32_t end = g_destroy_manager.timers.size();
    while(i < end) {
        destroy_timer_t timer = g_destroy_manager.timers[i];
        ecs::EntityRef ref{timer.passport};
        if(ref.valid() && timer.delay > 0.0f) {
            timer.delay -= g_time_layers[timer.time_layer].dt;
            ++i;
            continue;
        }

        ecs::EntityApi e = ref.ent();
        if (e.has<Node>()) {
            destroyNode(e);
        } else {
            ecx_destroy(e.index);
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
//        if (e.isAlive()) {
//            if (e.has<Node>()) {
//                destroyNode(e);
//            } else {
//                ecs::destroy(e);
//            }
//        }
//    }
}
