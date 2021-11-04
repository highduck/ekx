#include "NodeEvents.hpp"
#include "Node.hpp"

namespace ek {

void dispatch_broadcast(ecs::EntityApi e, const NodeEventData& data) {
    if (e.has<NodeEventHandler>()) {
        e.get<NodeEventHandler>().emit(data);
    }
    eachChild(e, [&data](ecs::EntityApi child) {
        dispatch_broadcast(child, data);
    });
}

void dispatch_bubble(ecs::EntityApi e, const NodeEventData& data) {
    auto it = e;
    while (it && it.isAlive()) {
        auto* eh = it.tryGet<NodeEventHandler>();
        if (eh) {
            eh->emit(data);
        }
        it = it.get<Node>().parent;
    }
}

}