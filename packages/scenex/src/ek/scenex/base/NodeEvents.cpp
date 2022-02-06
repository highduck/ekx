#include "NodeEvents.hpp"
#include "Node.hpp"

namespace ek {

void dispatch_broadcast(ecs::EntityApi e, const NodeEventData& data) {
    NodeEventHandler* ev = e.tryGet<NodeEventHandler>();
    if (ev) {
        data.receiver = e.index;
        ev->emit(data);
    }
    auto it = e.get<Node>().child_first;
    while (it) {
        dispatch_broadcast(it, data);
        it = it.get<Node>().sibling_next;
    }
}

void dispatch_bubble(ecs::EntityApi e, const NodeEventData& data) {
    auto it = e;
    while (it && it.is_alive()) {
        auto* eh = it.tryGet<NodeEventHandler>();
        if (eh) {
            data.receiver = it.index;
            eh->emit(data);
        }
        it = it.get<Node>().parent;
    }
}

}