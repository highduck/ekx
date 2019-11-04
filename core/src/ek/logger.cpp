#include "logger.hpp"

#include <cstdarg>

#if defined(__ANDROID__)

#include <android/log.h>

#else

#include <iostream>

#endif

namespace ek::logger {

#define RESET   "\033[0m"
#define BLACK   "\033[30m"      /* Black */
#define RED     "\033[31m"      /* Red */
#define GREEN   "\033[32m"      /* Green */
#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

const char* log_tag_ = "EK";
uint8_t log_filter_mask_ = static_cast<uint8_t>(verbosity_t::all);

void write(verbosity_t verbosity, source_location_t location, const char* message) noexcept {
    if ((log_filter_mask_ & static_cast<uint32_t>(verbosity)) == 0) {
        return;
    }

#if defined(__ANDROID__)
    int priority = 0;
    switch (verbosity) {
        case verbosity_t::info:
            priority = ANDROID_LOG_INFO;
            break;
        case verbosity_t::warning:
            priority = ANDROID_LOG_WARN;
            break;
        case verbosity_t::error:
            priority = ANDROID_LOG_ERROR;
            break;
        case verbosity_t::debug:
            priority = ANDROID_LOG_DEBUG;
            break;
        case verbosity_t::trace:
            priority = ANDROID_LOG_VERBOSE;
            break;
        default:
            return;
    }
    __android_log_write(priority, log_tag_, message);

#else

    switch (verbosity) {
        case verbosity_t::info:
            std::cout << BLUE "[i] ";
            break;
        case verbosity_t::warning:
            std::cout << BOLDYELLOW "[WARNING] ";
            break;
        case verbosity_t::error:
            std::cout << BOLDRED "[ERROR] ";
            break;
        case verbosity_t::debug:
            std::cout << CYAN "[d] ";
            break;
        case verbosity_t::trace:
            std::cout << WHITE "[t] ";
            break;
        default:
            return;
    }

    std::cout << message << RESET << std::endl;

    if (location.file) {
        std::cout << '\t' << location << std::endl;
    }
#endif
}
//
//void print(verbosity_t verbosity, source_location_t location, const char* format, ...) noexcept {
//    if ((log_filter_mask_ & static_cast<uint32_t>(verbosity)) == 0) {
//        return;
//    }
//
//    va_list args;
//    va_start(args, format);
//    char buf[256];
//    vsnprintf(buf, sizeof(buf), format, args);
//    write(verbosity, location, buf);
//    va_end(args);
//}

log_stream_t::log_stream_t(verbosity_t verbosity, source_location_t location) noexcept
        : verbosity_{verbosity},
          location_{location} {
}

log_stream_t::~log_stream_t() {
    write(verbosity_, location_, ss_.str().data());
}

void log_stream_t::operator()(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    char buf[256];
    vsnprintf(buf, sizeof(buf), format, args);
    ss_ << buf;
    va_end(args);
}

}