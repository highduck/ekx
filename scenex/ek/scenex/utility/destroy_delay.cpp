#include "destroy_delay.h"

#include <ek/scenex/components/node_t.h>

namespace ek {

void destroy_delayed_entities(float dt) {
    static std::vector<ecs::entity> destroy_queue;
    for (auto e : ecs::view<destroy_delay_t>()) {
        auto& c = ecs::get<destroy_delay_t>(e);
        c.delay -= dt;
        if (c.delay <= 0.0f) {
            destroy_queue.emplace_back(e);
        }
    }
    for (auto e : destroy_queue) {
        if (ecs::valid(e)) {
            if (ecs::has<node_t>(e)) {
                erase_node_component(e);
            }
            ecs::destroy(e);
        }
    }
    destroy_queue.clear();
}

}