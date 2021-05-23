#pragma once

#include "../config.hpp"
#include "SourceLocation.hpp"
#include "Verbosity.hpp"
#include <sstream>
#include "../time/Stopwatch.hpp"

#ifndef __printflike
#define __printflike(fmtarg, firstvararg)
#endif

namespace ek {

// log stream is temp object which collect and format message and
// on the end of life spawn this message to the log system

class NullLogStream {
public:
    inline explicit NullLogStream(Verbosity, SourceLocation) noexcept {}

    inline void operator()(const char*, ...) noexcept {}

    template<typename T>
    inline NullLogStream& operator<<(const T&) noexcept { return *this; }

    inline NullLogStream& operator<<(std::ostream& (*)(std::ostream&)) noexcept { return *this; }
};

class LogStream {
public:
    explicit LogStream(Verbosity verbosity, SourceLocation loc = {}) noexcept;

    ~LogStream();

    void operator()(const char* format, ...) noexcept __printflike(2, 3);

    template<typename T>
    LogStream& operator<<(const T& t) noexcept {
        ss_ << t;
        return *this;
    }

    // std::endl and other iomanip:s.
    LogStream& operator<<(std::ostream& (* f)(std::ostream&)) noexcept {
        f(ss_);
        return *this;
    }

private:
    Verbosity verbosity_;
    SourceLocation location_;
    std::ostringstream ss_;
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
        LogStream{_verbosity, _location}("%s: %0.3f ms", _label, _timer.readMillis());
    }
};

#ifdef EK_LOG_DEBUG_OUTPUT

#define EK_TRACE ek::LogStream{ek::Verbosity::Trace,   EK_CURRENT_LOCATION}
#define EK_DEBUG ek::LogStream{ek::Verbosity::Debug,   EK_CURRENT_LOCATION}

#else

#define EK_TRACE ek::NullLogStream{ek::Verbosity::Trace,   EK_CURRENT_LOCATION}
#define EK_DEBUG ek::NullLogStream{ek::Verbosity::Debug,   EK_CURRENT_LOCATION}

#endif

#define EK_INFO  ek::LogStream{ek::Verbosity::Info,    EK_CURRENT_LOCATION}
#define EK_WARN  ek::LogStream{ek::Verbosity::Warning, EK_CURRENT_LOCATION}
#define EK_ERROR ek::LogStream{ek::Verbosity::Error,   EK_CURRENT_LOCATION}

#ifdef EK_CONFIG_PROFILING
#define EK_PROFILE_SCOPE(label) ek::ScopedTimeMeter _time_meter_label{#label, EK_CURRENT_LOCATION, ek::Verbosity::Trace}
#else
#define EK_PROFILE_SCOPE(label) ((void)0)
#endif


}