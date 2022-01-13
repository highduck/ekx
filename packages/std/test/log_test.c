#include <doctest.h>
#include <ek/log.h>

TEST_SUITE_BEGIN("log");

TEST_CASE("print") {
    log_init();
    {
        log_print(LOG_LEVEL_DEBUG, nullptr, 0, "hello");
        EK_DEBUG("~Hey! => %d", 223);
        EK_DEBUG("simple digits: %d", 54);

        EK_DEBUG("macro");
        EK_DEBUG("and formatting %f", 4.0f);

        const char* p = "some/path/to/some/goodies";
        EK_INFO("%s", p);
        EK_DEBUG("%d", 2);
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
