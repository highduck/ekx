#include "NodeEvents.hpp"
#include "Node.hpp"

namespace ek {

void dispatch_broadcast(ecs::entity e, const NodeEventData& data) {
    if (e.has<NodeEventHandler>()) {
        e.get<NodeEventHandler>().emit(data);
    }
    eachChild(e, [&data](ecs::entity child) {
        dispatch_broadcast(child, data);
    });
}

void dispatch_bubble(ecs::entity e, const NodeEventData& data) {
    auto it = e;
    while (it && it.valid()) {
        auto* eh = it.tryGet<NodeEventHandler>();
        if (eh) {
            eh->emit(data);
        }
        it = it.get<Node>().parent;
    }
}

}