#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest.h>

int main(int argc, char** argv) {
    doctest::Context context{argc, argv};
    // don't break in the debugger when assertions fail
//    context.setOption("no-breaks", true);
    return context.run();
}