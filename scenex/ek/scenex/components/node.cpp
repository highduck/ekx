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
    return nullptr;
}

int node_t::findDepth(ecs::entity e) {
    int depth = 0;
    auto it = e.get<node_t>().parent;
    while (it) {
        ++depth;
        it = it.get<node_t>().parent;
    }
    return depth;
}

ecs::entity node_t::findLowerCommonAncestor(ecs::entity e1, ecs::entity e2) {
    auto depth1 = node_t::findDepth(e1);
    auto depth2 = node_t::findDepth(e2);
    auto it1 = e1;
    auto it2 = e2;
    while (depth1 > depth2) {
        it1 = it1.get<node_t>().parent;
        --depth1;
    }
    while (depth2 < depth1) {
        it2 = it2.get<node_t>().parent;
        --depth2;
    }
    while (it1 != it2) {
        it1 = it1.get<node_t>().parent;
        it2 = it2.get<node_t>().parent;
        if (!it1 || !it2) {
            return nullptr;
        }
    }
    return it1;
}

}