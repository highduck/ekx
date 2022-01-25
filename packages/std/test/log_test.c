#include <doctest.h>
#include <ek/log.h>

TEST_SUITE_BEGIN("log");

TEST_CASE("print") {
    log_init();
    {
        log_print(LOG_LEVEL_DEBUG, nullptr, 0, "hello");
        log_debug("~Hey! => %d", 223);
        log_debug("simple digits: %d", 54);

        log_debug("macro");
        log_debug("and formatting %f", 4.0f);

        const char* p = "some/path/to/some/goodies";
        log_info("%s", p);
        log_debug("%d", 2);
    }
    //log_term();


    //log_init();
    log_warn("WARNING!");
    log_tick();
    log_error("ERROR! %d", 233);
    log_tick();
    log_info("Hello, World!");
    log_tick();
}

TEST_SUITE_END();
