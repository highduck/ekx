#include "Interactive.hpp"

namespace ek {

void Interactive::handle(PointerEvent event) {
    switch (event) {
        case PointerEvent::Down:
            pushed = true;
            onEvent.emit(event);
            break;
        case PointerEvent::Up: {
            bool shouldBeClicked = pushed && over;
            pushed = false;
            onEvent.emit(event);

            if (shouldBeClicked) {
                onEvent.emit(PointerEvent::Tap);
            }
        }
            break;
        case PointerEvent::Over:
            over = true;
            onEvent.emit(event);
            break;
        case PointerEvent::Out:
            onEvent.emit(event);
            over = false;
            pushed = false;
            break;
        case PointerEvent::Tap:
            break;
    }
}

}