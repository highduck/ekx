#include "NodeEvents.hpp"
#include "Node.hpp"

namespace ek {

inline void send_event(entity_t e, const NodeEventData& data) {
    EK_ASSERT(is_entity(e));
    auto* ev = ecs::try_get<NodeEventHandler>(e);
    if (ev) {
        data.receiver = e;
        ev->emit(data);
    }
}

void dispatch_broadcast(entity_t e, const NodeEventData& data) {
    send_event(e, data);
    e = get_first_child(e);
    while (e.id) {
        dispatch_broadcast(e, data);
        e = get_next_child(e);
    }
}

void dispatch_bubble(entity_t e, const NodeEventData& data) {
    while (e.id) {
        send_event(e, data);
        e = get_parent(e);
    }
}

}