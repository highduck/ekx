#include "DestroyTimer.hpp"

#include <ek/scenex/base/Node.hpp>
#include <ek/timers.hpp>

namespace ek {

void destroy_delay(ecs::entity e, float delay, TimeLayer timer) {
    e.reassign<DestroyTimer>(delay, timer);
}

void destroy_children_delay(ecs::entity e, float delay, TimeLayer timer) {
    auto it = e.get<Node>().child_first;
    while (it) {
        it.reassign<DestroyTimer>(delay, timer);
        it = it.get<Node>().sibling_next;
    }
}

void DestroyTimer::updateAll() {
    static std::vector<ecs::entity> destroy_queue;
    for (auto e : ecs::view<DestroyTimer>()) {
        auto& c = e.get<DestroyTimer>();
        c.delay -= c.timer->dt;
        if (c.delay <= 0.0f) {
            destroy_queue.emplace_back(e);
        }
    }
    for (auto e : destroy_queue) {
        if (e.valid()) {
            if (e.has<Node>()) {
                destroyNode(e);
            } else {
                ecs::destroy(e);
            }
        }
    }
    destroy_queue.clear();
}
}