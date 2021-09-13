#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include <ek/app/app.hpp>
#include <ek/Arguments.hpp>

using namespace ek::app;

TEST_CASE("dummy_test, pass") {

}

namespace ek {

void main() {
    doctest::Context context;
    auto args = Arguments::current;
    context.applyCommandLine(args.argc, args.argv);

    int res = context.run(); // run
    if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        exit(res);          // propagate the result of the tests
}

}

