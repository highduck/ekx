#pragma once

#include "../config.hpp"

namespace ek {

enum class Verbosity {
    Trace = 0,
    Debug = 1,
    Info = 2,
    Warning = 3,
    Error = 4
};

struct SourceLocation final {
    const char* file = nullptr;
    unsigned line = 0;
};

struct LogMessage final {
    Verbosity verbosity = Verbosity::Trace;
    SourceLocation location = {};
    const char* message = nullptr;
    unsigned char frameHash = 0;
};

typedef void(*LogSink)(const LogMessage& message);

}

#ifdef EK_SOURCE_LOCATION_ENABLED

#define EK_CURRENT_LOCATION ek::SourceLocation{__FILE__, __LINE__}

#else

#define EK_CURRENT_LOCATION ek::SourceLocation{}

#endif