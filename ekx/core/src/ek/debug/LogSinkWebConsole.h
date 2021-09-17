#pragma once

#if defined(__EMSCRIPTEN__)

#include "LogMessage.hpp"
#include <emscripten.h>

namespace ek {

void logSinkWebConsole(const LogMessage& message) {
    const auto level = (int) message.verbosity;
    const char* text = message.message;
    EM_ASM(console[(["trace", "log", "info", "warn", "error"])[($0)]](UTF8ToString($1)), level, text);
}

}

#endif