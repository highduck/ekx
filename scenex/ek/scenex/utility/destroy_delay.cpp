#include "destroy_delay.hpp"

#include <ek/scenex/components/node.hpp>
#include <ek/scenex/game_time.hpp>

namespace ek {

void destroy_delayed_entities(float dt) {
    static std::vector<ecs::entity> destroy_queue;
    for (auto e : ecs::view<destroy_delay_t>()) {
        auto& c = e.get<destroy_delay_t>();
        c.delay -= dt;
        if (c.delay <= 0.0f) {
            destroy_queue.emplace_back(e);
        }
    }
    for (auto e : destroy_queue) {
        if (e.valid()) {
            if (e.has<node_t>()) {
                destroy_node(e);
            } else {
                ecs::destroy(e);
            }
        }
    }
    destroy_queue.clear();
}

void destroy_delay(ecs::entity e, float delay, TimeLayer timer) {
    e.reassign<destroy_delay_t>(delay, timer);
}

void destroy_children_delay(ecs::entity e, float delay, TimeLayer timer) {
    auto it = e.get<node_t>().child_first;
    while (it) {
        it.reassign<destroy_delay_t>(delay, timer);
        it = it.get<node_t>().sibling_next;
    }
}

}