#include <doctest.h>

#include "LogSystem.hpp"
#include "LogStream.hpp"
#include <ek/util/Path.hpp>

using namespace ek;

TEST_CASE("logger_test simple") {
    memory::initialize();
    LogSystem::initialize();
    {
        LogSystem::instance().write(Verbosity::Debug, {}, "hello");
        LogStream{Verbosity::Debug} << "~Hey! => " << 223;
        LogStream{Verbosity::Debug, {}}("simple digits: %d", 54);

        EK_DEBUG << "macro" << 9;
        EK_DEBUG("and formatting %f", 4.0f);

        path_t p{"some/path/to/some/goodies"};
        EK_INFO << p;
        LogStream{Verbosity::Debug, EK_CURRENT_LOCATION}("%d", 2);
    }
    LogSystem::shutdown();
    memory::shutdown();
}