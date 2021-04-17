#include "logger.hpp"

#include <cstdarg>

#if defined(__ANDROID__)

#include <android/log.h>

#else

#include <cstdio>
#include <ctime>
#include <sys/time.h>

#endif

#if defined(__APPLE__)
#include <TargetConditionals.h>
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

const char* _tag = "EK";
uint8_t _filter = static_cast<uint8_t>(verbosity_t::all);
uint8_t _frame = 0;

void nextFrame() {
    ++_frame;
}

void write(verbosity_t verbosity,
           [[maybe_unused]] source_location_t location,
           const char* message) noexcept {
    if ((_filter & static_cast<uint32_t>(verbosity)) == 0) {
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

#ifndef NDEBUG
    __android_log_print(priority, _tag, "@%02hhX: %s", _frame, message);
#else
    // for release reduce allocations for printing
    __android_log_write(priority, _tag, message);
#endif

#elif TARGET_OS_IOS
    const char* prefix = nullptr;
    switch (verbosity) {
        case verbosity_t::info:
            prefix = "[i] ";
            break;
        case verbosity_t::warning:
            prefix = "[W] ";
            break;
        case verbosity_t::error:
            prefix = "[E] ";
            break;
        case verbosity_t::debug:
            prefix = "[d] ";
            break;
        case verbosity_t::trace:
            prefix = "[t] ";
            break;
        default:
            return;
    }
    printf("%02hhX/%s%s\n", _frame, prefix, message);
    if (location.file) {
        printf("\t%s:%d\n", location.file, location.line);
    }

#else
    char time[24];
    char usec[8];

    struct timeval tmnow;
    struct tm *tm;
    gettimeofday(&tmnow, nullptr);
#if defined(_WIN32) || defined(_WIN64)
    tm = _localtime32(&tmnow.tv_sec);
#else
    tm = localtime(&tmnow.tv_sec);
#endif
    strftime(time, sizeof(time), "%d/%m/%Y %H:%M:%S", tm);
    snprintf(usec, sizeof(usec), "%03d/%02hhX", static_cast<int32_t>(tmnow.tv_usec / 1000), _frame);
    const char* prefix = nullptr;
    switch (verbosity) {
        case verbosity_t::info:
            prefix = BLUE "[i] ";
            break;
        case verbosity_t::warning:
            prefix = BOLDYELLOW "[W] ";
            break;
        case verbosity_t::error:
            prefix = BOLDRED "[E] ";
            break;
        case verbosity_t::debug:
            prefix = CYAN "[d] ";
            break;
        case verbosity_t::trace:
            prefix = WHITE "[t] ";
            break;
        default:
            return;
    }

    printf("%s%s.%s: %s" RESET "\n", prefix, time, usec, message);
    if (location.file) {
        printf("\t%s:%d\n", location.file, location.line);
    }
#endif
}

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