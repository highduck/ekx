#pragma once

#include "LogSystem.hpp"
#include "LogStream.hpp"

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

std::ostream& operator<<(std::ostream& out, const ek::SourceLocation& location) {
    if (location.file) {
        out << location.file;
        if (location.line) {
            out << ':' << location.line;
        }
    }
    return out;
}

namespace ek {

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

LogSystem::LogSystem() :
        allocator{memory::systemAllocator, "log-system"} {

}

void LogSystem::setVerbosity(Verbosity filter_) {
    filter = filter_;
}

void LogSystem::nextFrame() {
    ++frameHash;
}

void LogSystem::write(Verbosity verbosity, SourceLocation location, const char* message) noexcept {
    if ((static_cast<unsigned>(filter) & static_cast<unsigned>(verbosity)) == 0) {
        return;
    }

#if defined(__ANDROID__)
    int priority = 0;
    switch (verbosity) {
        case Verbosity::Info:
            priority = ANDROID_LOG_INFO;
            break;
        case Verbosity::Warning:
            priority = ANDROID_LOG_WARN;
            break;
        case Verbosity::Error:
            priority = ANDROID_LOG_ERROR;
            break;
        case Verbosity::Debug:
            priority = ANDROID_LOG_DEBUG;
            break;
        case Verbosity::Trace:
            priority = ANDROID_LOG_VERBOSE;
            break;
        default:
            return;
    }

#ifndef NDEBUG
    __android_log_print(priority, tag, "@%02hhX: %s", frameHash, message);
#else
    // for release reduce allocations for printing
    __android_log_write(priority, tag, message);
#endif

#elif TARGET_OS_IOS
    const char* prefix = nullptr;
    switch (verbosity) {
        case Verbosity::Info:
            prefix = "[i] ";
            break;
        case Verbosity::Warning:
            prefix = "[W] ";
            break;
        case Verbosity::Error:
            prefix = "[E] ";
            break;
        case Verbosity::Debug:
            prefix = "[d] ";
            break;
        case Verbosity::Trace:
            prefix = "[t] ";
            break;
        default:
            return;
    }
    printf("%02hhX/%s%s\n", frameHash, prefix, message);
    if (location.file) {
        printf("\t%s:%d\n", location.file, location.line);
    }

#else
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
    strftime(time, sizeof(time), "%d/%m/%Y %H:%M:%S", tm);
    snprintf(usec, sizeof(usec), "%03d/%02hhX", static_cast<int32_t>(tmnow.tv_usec / 1000), frameHash);
    const char* prefix = nullptr;
    switch (verbosity) {
        case Verbosity::Info:
            prefix = BLUE "[i] ";
            break;
        case Verbosity::Warning:
            prefix = BOLDYELLOW "[W] ";
            break;
        case Verbosity::Error:
            prefix = BOLDRED "[E] ";
            break;
        case Verbosity::Debug:
            prefix = CYAN "[d] ";
            break;
        case Verbosity::Trace:
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

    if (!sinks.empty()) {
        LogMessage msg;
        msg.verbosity = verbosity;
        msg.frameHash = frameHash;
        msg.location = location;
        msg.message = message;
        for (auto* sink : sinks) {
            sink->onMessageWrite(msg);
        }
    }
}

LogSystem* gLogSystem = nullptr;

void LogSystem::initialize() {
    if (!gLogSystem) {
        gLogSystem = new LogSystem();
    }
}

void LogSystem::shutdown() {
    if (gLogSystem) {
        delete gLogSystem;
    }
}

LogSystem& LogSystem::instance() {
    return *gLogSystem;
}

LogStream::LogStream(Verbosity verbosity, SourceLocation location) noexcept:
        verbosity_{verbosity},
        location_{location} {
}

LogStream::~LogStream() {
    gLogSystem->write(verbosity_, location_, ss_.str().data());
}

void LogStream::operator()(const char* format, ...) noexcept {
    va_list args;
    va_start(args, format);
    char buf[256];
    vsnprintf(buf, sizeof(buf), format, args);
    ss_ << buf;
    va_end(args);
}

}
