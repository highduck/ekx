#include "node.hpp"

namespace ek {

ecs::entity get_child_at(ecs::entity e, int index) {
    int i = 0;
    auto it = ecs::get<node_t>(e).child_first;
    while (it) {
        if (i == index) {
            return it;
        }
        ++i;
        it = ecs::get<node_t>(it).sibling_next;
    }
    return ecs::null;
}

}