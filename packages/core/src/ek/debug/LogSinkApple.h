#pragma once

#if defined(__APPLE__)

#include <CoreFoundation/CoreFoundation.h>
#include "LogMessage.hpp"

extern "C" {
extern void NSLog(CFStringRef format, ...);
}

namespace ek {

inline void logSinkApple(const LogMessage& message) {
    const char* prefix = nullptr;
    switch (message.verbosity) {
        case Verbosity::Info:
            prefix = "i";
            break;
        case Verbosity::Warning:
            prefix = "W";
            break;
        case Verbosity::Error:
            prefix = "E";
            break;
        case Verbosity::Debug:
            prefix = "d";
            break;
        case Verbosity::Trace:
            prefix = "t";
            break;
        default:
            return;
    }
    if (message.location.file) {
        NSLog(CFSTR("%s^%02hhX %s %s:%d"), prefix, message.frameHash, message.message, message.location.file,
              message.location.line);
    } else {
        NSLog(CFSTR("%s^%02hhX %s"), prefix, message.frameHash, message.message);
    }
}

}

#endif