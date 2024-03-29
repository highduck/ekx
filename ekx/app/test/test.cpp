#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

#include <ek/app/app.hpp>

TEST_CASE("dummy_test, pass") {

}

void ek::app::main() {
    doctest::Context context;
    context.applyCommandLine(g_app.argc, g_app.argv);

    int res = context.run(); // run
    //if(context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
        quit(res);          // propagate the result of the tests
}
