#pragma once

#include "../config.hpp"
#include "../time/Stopwatch.hpp"
#include "LogMessage.hpp"

#ifndef __printflike
#define __printflike(fmtarg, firstvararg)
#endif

namespace ek {

class LogSystem {
public:
    static void initialize();
    static void shutdown();

    static void nextFrame();

    static void log(Verbosity, SourceLocation, const char*);

    static void format(Verbosity, SourceLocation, const char*, ...) __printflike(3, 4);

    static void write(Verbosity verbosity, SourceLocation location, const char* message);

    static void setFilter(uint8_t mask);

    [[nodiscard]]
    static bool filter(Verbosity level);

    inline static LogSink sinks[8]{};
    inline static int sinksCount = 0;
    inline static uint8_t filterMask = 0xFF;
    inline static uint8_t frameHash = 0;

    static void addLogSink(LogSink logSink);
};

class ScopedTimeMeter final {
private:
    Stopwatch _timer;
    const char* _label;
    SourceLocation _location;
    Verbosity _verbosity;
public:
    explicit ScopedTimeMeter(const char* label,
                             SourceLocation location = {},
                             Verbosity verbosity = Verbosity::Trace) :
            _timer{},
            _label{label},
            _location{location},
            _verbosity{verbosity} {
    }

    ~ScopedTimeMeter() {
        LogSystem::format(_verbosity, _location, "%s: %d ms", _label, (int)_timer.readMillis());
    }
};

}


#ifdef EK_LOG_DEBUG_OUTPUT

#define EK_TRACE(x) ek::LogSystem::log(ek::Verbosity::Trace, EK_CURRENT_LOCATION, (x))
#define EK_DEBUG(x) ek::LogSystem::log(ek::Verbosity::Debug, EK_CURRENT_LOCATION, (x))
#define EK_TRACE_F(x, ...) ek::LogSystem::format(ek::Verbosity::Trace, EK_CURRENT_LOCATION, (x), __VA_ARGS__)
#define EK_DEBUG_F(x, ...) ek::LogSystem::format(ek::Verbosity::Debug, EK_CURRENT_LOCATION, (x), __VA_ARGS__)

#else

#define EK_TRACE(x) ((void)(0))
#define EK_DEBUG(x) ((void)(0))
#define EK_TRACE_F(x, ...) ((void)(0))
#define EK_DEBUG_F(x, ...) ((void)(0))

#endif

#define EK_INFO(x)  ek::LogSystem::log(ek::Verbosity::Info,    EK_CURRENT_LOCATION, (x))
#define EK_WARN(x)  ek::LogSystem::log(ek::Verbosity::Warning, EK_CURRENT_LOCATION, (x))
#define EK_ERROR(x) ek::LogSystem::log(ek::Verbosity::Error,   EK_CURRENT_LOCATION, (x))
#define EK_INFO_F(x, ...)  ek::LogSystem::format(ek::Verbosity::Info,    EK_CURRENT_LOCATION, (x), __VA_ARGS__)
#define EK_WARN_F(x, ...)  ek::LogSystem::format(ek::Verbosity::Warning, EK_CURRENT_LOCATION, (x), __VA_ARGS__)
#define EK_ERROR_F(x, ...) ek::LogSystem::format(ek::Verbosity::Error,   EK_CURRENT_LOCATION, (x), __VA_ARGS__)

#ifdef EK_CONFIG_PROFILING
#define EK_PROFILE_SCOPE(label) ek::ScopedTimeMeter _time_meter_label{#label, EK_CURRENT_LOCATION, ek::Verbosity::Trace}
#else
#define EK_PROFILE_SCOPE(label) ((void)0)
#endif