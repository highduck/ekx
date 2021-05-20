#include <gtest/gtest.h>
#include <ek/debug.hpp>
#include <ek/util/Path.hpp>

using namespace ek;

TEST(logger_test, simple) {
    LogStream{Verbosity::Debug} << "~Hey! => " << 223;
//    logger::print(logger::verbosity_t::debug, {}, "smiple digits: %d", 54);

    EK_DEBUG << "macro" << 9;
    EK_DEBUG("and formatting %f", 4.0f);

    path_t p{"some/path/to/some/goodies"};
    EK_INFO << p;
    LogStream{Verbosity::Debug, EK_CURRENT_LOCATION}("%d", 2);
}