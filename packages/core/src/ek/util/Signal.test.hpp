#include <unit.h>
#include "Signal.hpp"

SUITE(cxx_signals) {

    IT("basic") {
        using namespace ek;
        Signal<> event;

        int r = 0;

        event += [&r]() {
            r += 1;
        };

        event << [&r]() {
            r += 3;
        };

        event();
        event();

        REQUIRE_EQ(r, 5);
    }

    IT("resize") {
        using namespace ek;
        auto* event = new Signal<>();
        auto* event2 = new Signal<>();

        int r = 0;

        *event += [&]() {
            r += 3;
            *event2 = *event;
            delete event;
            event = event2;
            event2 = new Signal<>();
        };

        // this callback will miss, because source signal list will be deleted
        *event += [&r]() {
            r += 1;
        };

        event->emit();
        event->emit();

        REQUIRE_EQ(r, 6);

        delete event;
        delete event2;
    }

}
