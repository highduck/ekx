#include "interactive.hpp"

namespace ek {

void interactive_t::handle(InteractionEvent event) {
    switch (event) {
        case InteractionEvent::PointerDown:
            pushed = true;
            on_down();
            break;
        case InteractionEvent::PointerUp: {
            bool shouldBeClicked = pushed && over;
            pushed = false;
            on_up();

            if (shouldBeClicked) {
                on_clicked();
            }
        }
            break;
        case InteractionEvent::PointerOver:
            over = true;
            on_over();
            break;
        case InteractionEvent::PointerOut:
            on_out();
            over = false;
            pushed = false;
            break;
        case InteractionEvent::Click:
            break;
    }
}

}