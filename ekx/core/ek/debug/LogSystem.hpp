#pragma once

#include "SourceLocation.hpp"
#include "Verbosity.hpp"
#include "../Allocator.hpp"
#include "../ds/Array.hpp"
#include "LogSink.hpp"
#include "LogMessage.hpp"

namespace ek {

class LogSystem {
public:
    LogSystem();

    void nextFrame();

    void write(Verbosity verbosity, SourceLocation location, const char* message) noexcept;

    void setVerbosity(Verbosity filter = Verbosity::All);

    const char* tag = "EK";
    Verbosity filter = Verbosity::All;
    unsigned char frameHash = 0;
    ProxyAllocator allocator;
    Array<LogSink*> sinks{};

    static void setup();
    static LogSystem& instance();
};

}