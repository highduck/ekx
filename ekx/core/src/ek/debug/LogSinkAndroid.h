#pragma once

#if defined(__ANDROID__)

#include "LogMessage.hpp"
#include <android/log.h>

namespace ek {

inline void logSinkAndroid(const LogMessage& message) {
    int priority = 0;
    switch (message.verbosity) {
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
    const char* tag = "ekx";
#ifndef NDEBUG
    __android_log_print(priority, tag, "@%02hhX: %s", message.frameHash, message.message);
#else
    // for release reduce allocations for printing
    __android_log_write(priority, tag, message.message);
#endif
}

}

#endif