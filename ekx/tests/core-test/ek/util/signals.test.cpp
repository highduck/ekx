#include <ek/util/Signal.hpp>

#include <gtest/gtest.h>

using namespace ek;

TEST(signals, basic) {
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

        ASSERT_EQ(r, 5);
    }
    memory::shutdown();
}

TEST(signals, resize) {
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

        ASSERT_EQ(r, 8);

        delete event;
    }
    memory::shutdown();
}