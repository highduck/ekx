#include <gtest/gtest.h>
#include <ek/logger.hpp>
#include <ek/fs/path.hpp>

using namespace ek;

TEST(logger_test, simple) {
    logger::log_stream_t{logger::verbosity_t::debug} << "~Hey! => " << 223;
//    logger::print(logger::verbosity_t::debug, {}, "smiple digits: %d", 54);

    EK_DEBUG << "macro" << 9;
    EK_DEBUG("and formatting %f", 4.0f);

    path_t p{"some/path/to/some/goodies"};
    EK_INFO << p;
    ::ek::logger::log_stream_t{::ek::logger::verbosity_t::debug,   EK_CURRENT_LOCATION}("%d", 2);
}