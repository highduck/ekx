#pragma once

#include "LogMessage.hpp"
#include <cstdio>
#include <ctime>
#include <sys/time.h>

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

namespace ek {

inline void logSinkSystem(const LogMessage& message) {
    char time[24];
    char usec[8];

    struct timeval tmnow;
    struct tm* tm;
    gettimeofday(&tmnow, nullptr);
#if defined(_WIN32) || defined(_WIN64)
    tm = _localtime32(&tmnow.tv_sec);
#else
    tm = localtime(&tmnow.tv_sec);
#endif
    strftime(time, sizeof(time), "%Y-%m-%d %H:%M:%S", tm);
    snprintf(usec, sizeof(usec), "%03d+%02hhX", static_cast<int32_t>(tmnow.tv_usec / 1000), message.frameHash);
    const char* prefix = nullptr;
    switch (message.verbosity) {
        case Verbosity::Info:
            prefix = BLUE "[i]";
            break;
        case Verbosity::Warning:
            prefix = BOLDYELLOW "[W]";
            break;
        case Verbosity::Error:
            prefix = BOLDRED "[E]";
            break;
        case Verbosity::Debug:
            prefix = CYAN "[d]";
            break;
        case Verbosity::Trace:
            prefix = WHITE "[t]";
            break;
        default:
            return;
    }

    char buffer[1024];
    stbsp_snprintf(buffer, 1024, "%s.%s %s %s" RESET, time, usec, prefix, message.message);
    puts(buffer);
    if (message.location.file) {
        stbsp_snprintf(buffer, 1024, "\t%s:%d", message.location.file, message.location.line);
        puts(buffer);
    }
}

}
