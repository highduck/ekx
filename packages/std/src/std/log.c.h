#include <ek/log.h>
#include <ek/assert.h>
#include <ek/string.h>

enum {
    LOG_CALLBACKS_MAX = 4
};

static struct log_ctx_ {
    log_callback_t callbacks[LOG_CALLBACKS_MAX];
    uint32_t callbacks_num;
    uint16_t frame;
    uint16_t mask;
} g_log;

void log_print(log_level_t level, const char* file, int line, const char* fmt, ...) {
    if (g_log.callbacks_num == 0 || (g_log.mask & (1 << level)) == 0) {
        return;
    }
    char text[1024];
    va_list args;
    va_start(args, fmt);
    stbsp_vsnprintf(text, sizeof(text), fmt, args);
    va_end(args);

    const log_msg_t msg = {
            .text = text,
            .level = level,
            .file = file,
            .line = line,
            .frame = g_log.frame
    };
    for (uint32_t i = 0; i < g_log.callbacks_num; ++i) {
        g_log.callbacks[i](msg);
    }
}

void log_tick(void) {
    ++g_log.frame;
}

void log_add_sink(log_callback_t cb) {
    EK_ASSERT(g_log.callbacks_num < LOG_CALLBACKS_MAX);
    g_log.callbacks[g_log.callbacks_num++] = cb;
}

void log_set_levels(uint16_t mask) {
    g_log.mask = mask;
}

#if defined(__EMSCRIPTEN__)

#include <emscripten.h>

static void log__default(const log_msg_t msg) {
    EM_ASM({
               console[(["trace", "log", "info", "warn", "error", "error"])[($0)]](UTF8ToString($1))
           }, msg.level, msg.text);
}

#elif defined(__ANDROID__)

#include <android/log.h>

static void log__default(log_msg_t msg) {
  // android
  static const int priorities[] = {
          ANDROID_LOG_VERBOSE,
          ANDROID_LOG_DEBUG,
          ANDROID_LOG_INFO,
          ANDROID_LOG_WARN,
          ANDROID_LOG_ERROR,
          ANDROID_LOG_ERROR
  };
  const int priority = priorities[msg.level];

    const char* tag = "ekx";
#ifndef NDEBUG
    __android_log_print(priority, tag, "@%02hhX: %s", (uint8_t)(msg.frame & 0xFF), msg.text);
#else
    // for release reduce allocations for printing
    __android_log_write(priority, tag, msg.text);
#endif
}

#else // __ANDROID__

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

#define RESET   "\033[0m"
//#define BLACK   "\033[30m"      /* Black */
//#define RED     "\033[31m"      /* Red */
//#define GREEN   "\033[32m"      /* Green */
//#define YELLOW  "\033[33m"      /* Yellow */
#define BLUE    "\033[34m"      /* Blue */
//#define MAGENTA "\033[35m"      /* Magenta */
#define CYAN    "\033[36m"      /* Cyan */
#define WHITE   "\033[37m"      /* White */
//#define BOLDBLACK   "\033[1m\033[30m"      /* Bold Black */
#define BOLDRED     "\033[1m\033[31m"      /* Bold Red */
//#define BOLDGREEN   "\033[1m\033[32m"      /* Bold Green */
#define BOLDYELLOW  "\033[1m\033[33m"      /* Bold Yellow */
//#define BOLDBLUE    "\033[1m\033[34m"      /* Bold Blue */
//#define BOLDMAGENTA "\033[1m\033[35m"      /* Bold Magenta */
//#define BOLDCYAN    "\033[1m\033[36m"      /* Bold Cyan */
//#define BOLDWHITE   "\033[1m\033[37m"      /* Bold White */

static void log__default(log_msg_t msg) {
    // system
    char time[24];
    char usec[8];

    struct timeval tmnow;
    struct tm* tm;
    gettimeofday(&tmnow, 0);
#if defined(_WIN32) || defined(_WIN64)
    tm = _localtime32(&tmnow.tv_sec);
#else
    tm = localtime(&tmnow.tv_sec);
#endif
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", tm);
    const int32_t millis = (int32_t) (tmnow.tv_usec / 1000);
    const uint8_t frame = (uint8_t) (msg.frame & 0xFFu);
    stbsp_snprintf(usec, sizeof(usec), "%03d+%02hhX", millis, frame);
    static const char* prefixes[] = {
            WHITE "[t]", // TRACE
            CYAN "[d]", // DEBUG
            BLUE "[i]", // INFO
            BOLDYELLOW "[W]", // WARN
            BOLDRED "[E]", // ERROR
            BOLDRED "[F]", // FATAL
    };
    EK_ASSERT(msg.level >= 0 && msg.level < 6);
    const char* prefix = prefixes[msg.level];

    char buffer[1024];
    stbsp_snprintf(buffer, 1024, "%s.%s %s %s" RESET, time, usec, prefix, msg.text);
    puts(buffer);
    if (msg.file && msg.file[0] != '\0') {
        stbsp_snprintf(buffer, 1024, "\t%s:%d", msg.file, msg.line);
        puts(buffer);
    }
}

#endif // SYSTEM

void log_init(void) {
    memset(&g_log, 0, sizeof(g_log));
    g_log.mask = 0xFFFF;
    log_add_sink(log__default);
}

//#if defined(__APPLE__)
//
//#include <CoreFoundation/CoreFoundation.h>
//#include "LogMessage.hpp"
//
//extern "C" {
//extern void NSLog(CFStringRef format, ...);
//}
//
//namespace ek {
//
//        inline void logSinkApple(const LogMessage& message) {
//            const char* prefix = nullptr;
//            switch (message.verbosity) {
//                case Verbosity::Info:
//                    prefix = "i";
//                    break;
//                case Verbosity::Warning:
//                    prefix = "W";
//                    break;
//                case Verbosity::Error:
//                    prefix = "E";
//                    break;
//                case Verbosity::Debug:
//                    prefix = "d";
//                    break;
//                case Verbosity::Trace:
//                    prefix = "t";
//                    break;
//                default:
//                    return;
//            }
//            if (message.location.file) {
//                NSLog(CFSTR("%s^%02hhX %s %s:%d"), prefix, message.frameHash, message.message, message.location.file,
//                      message.location.line);
//            } else {
//                NSLog(CFSTR("%s^%02hhX %s"), prefix, message.frameHash, message.message);
//            }
//        }
//
//}
//
//#endif
