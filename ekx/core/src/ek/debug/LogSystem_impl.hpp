#pragma once

#include "LogSystem.hpp"

#define STB_SPRINTF_IMPLEMENTATION
#define STB_SPRINTF_NOFLOAT

#include <stb/stb_sprintf.h>

#if defined(__ANDROID__)

#include "LogSinkAndroid.h"

#elif defined(__EMSCRIPTEN__)

#include "LogSinkWebConsole.h"

//#elif defined(__APPLE__) && TARGET_IPHONE
//
//#include "LogSinkApple.h"
//
#else

#include "LogSinkSystem.h"

#endif

namespace ek {

void LogSystem::setFilter(uint8_t mask) {
    filterMask = mask;
}

void LogSystem::nextFrame() {
    ++frameHash;
}

void LogSystem::write(Verbosity verbosity, SourceLocation location, const char* message) {
    if (sinksCount > 0) {
        if (!filter(verbosity)) {
            return;
        }
        LogMessage msg;
        msg.verbosity = verbosity;
        msg.frameHash = frameHash;
        msg.location = location;
        msg.message = message;
        for (int i = 0; i < sinksCount; ++i) {
            sinks[i](msg);
        }
    }
}

void LogSystem::initialize() {
#if defined(__ANDROID__)
    addLogSink(logSinkAndroid);
#elif defined(__EMSCRIPTEN__)
    addLogSink(logSinkWebConsole);
//#elif defined(__APPLE__)
//    addLogSink(logSinkApple);
#else
    addLogSink(logSinkSystem);
#endif
}

void LogSystem::shutdown() {
    sinksCount = 0;
}

bool LogSystem::filter(Verbosity level) {
    return (filterMask & (1 << static_cast<unsigned>(level))) != 0;
}

void LogSystem::addLogSink(LogSink logSink) {
    if (sinksCount < 8) {
        sinks[sinksCount++] = logSink;
    } else {
        write(Verbosity::Error, {}, "log sinks limit");
    }
}

void LogSystem::log(Verbosity verbosity, SourceLocation location, const char* message) {
    write(verbosity, location, message);
}

void LogSystem::format(Verbosity verbosity, SourceLocation location, const char* format, ...) {
    if (filter(verbosity)) {
        va_list args;
        va_start(args, format);
        char buf[256];
        stbsp_vsnprintf(buf, 256, format, args);
        va_end(args);
        write(verbosity, location, buf);
    }
}

}
