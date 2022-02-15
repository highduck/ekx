#include "Interactive.hpp"
#include "NodeEvents.hpp"
#include "ek/scenex/2d/Button.hpp"

namespace ek {

void Interactive::handle(entity_t e, string_hash_t type) {
    NodeEventHandler* eh = ecs::try_get<NodeEventHandler>(e);
    bool shouldBeClicked = false;
    if (type == POINTER_EVENT_DOWN) {
        pushed = true;
        if (eh) eh->emit({type, e, {nullptr}, e});
        ev_down = true;
    } else if (type == POINTER_EVENT_UP) {
        shouldBeClicked = pushed && over;
        pushed = false;
        if (eh) eh->emit({type, e, {nullptr}, e});
    } else if (type == POINTER_EVENT_OVER) {
        over = true;
        if (eh) eh->emit({type, e, {nullptr}, e});

        ev_over = true;
    } else if (type == POINTER_EVENT_OUT) {
        over = false;
        pushed = false;
        if (eh) eh->emit({type, e, {nullptr}, e});
        ev_out = true;
    } else {
        return;
    }

    if (shouldBeClicked) {
        ev_tap = true;
        if(eh) {
            eh->emit({POINTER_EVENT_TAP, e, {nullptr}, e});
        }
    }

}

}