#include <doctest.h>

#include "Signal.hpp"

using namespace ek;

TEST_CASE("signals basic") {
    memory::initialize();
    {
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
    memory::shutdown();
}

TEST_CASE("signals resize") {
    memory::initialize();
    {
        auto* event = new Signal<>;
        Signal<> event2;

        int r = 0;

        *event += [&]() {
            r += 3;
            event2 = *event;
            delete event;
            event = new Signal<>;
            *event = event2;
        };

        *event += [&r]() {
            r += 1;
        };

        (*event)();
        (*event)();

        REQUIRE_EQ(r, 8);

        delete event;
    }
    memory::shutdown();
}