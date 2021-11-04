#include <doctest.h>

#include "LogSystem.hpp"

using namespace ek;

TEST_CASE("logger_test simple") {
    LogSystem::initialize();
    {
        LogSystem::write(Verbosity::Debug, {}, "hello");
        EK_DEBUG_F("~Hey! => %d", 223);
        EK_DEBUG_F("simple digits: %d", 54);

        EK_DEBUG("macro");
        EK_DEBUG_F("and formatting %f", 4.0f);

        const auto* p = "some/path/to/some/goodies";
        EK_INFO(p);
        EK_DEBUG_F("%d", 2);
    }
    LogSystem::shutdown();
}