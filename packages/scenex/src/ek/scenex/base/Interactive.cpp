#include "Interactive.hpp"
#include "NodeEvents.hpp"

namespace ek {

void Interactive::handle(entity_t e, string_hash_t type) {
    NodeEventHandler* eh = e ? ecs::EntityApi{e}.tryGet<NodeEventHandler>(): nullptr;
    bool shouldBeClicked = false;
    if (type == POINTER_EVENT_DOWN) {
        pushed = true;
        if (eh) eh->emit({type, e, {nullptr}, e});
    } else if (type == POINTER_EVENT_UP) {
        shouldBeClicked = pushed && over;
        pushed = false;
        if (eh) eh->emit({type, e, {nullptr}, e});
    } else if (type == POINTER_EVENT_OVER) {
        over = true;
        if (eh) eh->emit({type, e, {nullptr}, e});
    } else if (type == POINTER_EVENT_OUT) {
        // keep order for now, later maybe unified at the end of function
        if (eh) eh->emit({type, e, {nullptr}, e});
        over = false;
        pushed = false;
    } else {
        return;
    }

    if (eh && shouldBeClicked) {
        eh->emit({POINTER_EVENT_TAP, e, {nullptr}, e});
    }
}

}