#pragma once

#include <cstdint>
#include <sstream>
#include <ek/util/source_location.hpp>

#ifndef __printflike
#define __printflike(fmtarg, firstvararg)
#endif

namespace ek::logger {

enum class verbosity_t : uint8_t {
    none = 0x0u,

    trace = 0x1u,
    debug = 0x2u,
    info = 0x4u,
    warning = 0x8u,
    error = 0x10u,

    all = 0xFFu
};

void write(verbosity_t verbosity, source_location_t location, const char* message) noexcept;

void set_log_filter(verbosity_t filter = verbosity_t::all);

class null_log_stream_t {
public:
    inline explicit null_log_stream_t(verbosity_t, source_location_t) noexcept {}

    inline void operator()(const char*, ...) noexcept {}

    template<typename T>
    inline null_log_stream_t& operator<<(const T&) noexcept { return *this; }

    inline null_log_stream_t& operator<<(std::ostream& (*)(std::ostream&)) noexcept { return *this; }
};

class log_stream_t {
public:
    explicit log_stream_t(verbosity_t verbosity, source_location_t loc = {}) noexcept;

    ~log_stream_t();

    void operator()(const char* format, ...) noexcept __printflike(2, 3);

    template<typename T>
    log_stream_t& operator<<(const T& t) noexcept {
        ss_ << t;
        return *this;
    }

    // std::endl and other iomanip:s.
    log_stream_t& operator<<(std::ostream& (* f)(std::ostream&)) noexcept {
        f(ss_);
        return *this;
    }

private:
    verbosity_t verbosity_;
    source_location_t location_;
    std::ostringstream ss_;
};

#ifdef NDEBUG

#define EK_TRACE ::ek::logger::null_log_stream_t{::ek::logger::verbosity_t::trace,   EK_CURRENT_LOCATION}
#define EK_DEBUG ::ek::logger::null_log_stream_t{::ek::logger::verbosity_t::debug,   EK_CURRENT_LOCATION}

#else

#define EK_TRACE ::ek::logger::log_stream_t{::ek::logger::verbosity_t::trace,   EK_CURRENT_LOCATION}
#define EK_DEBUG ::ek::logger::log_stream_t{::ek::logger::verbosity_t::debug,   EK_CURRENT_LOCATION}

#endif

#define EK_INFO  ::ek::logger::log_stream_t{::ek::logger::verbosity_t::info,    EK_CURRENT_LOCATION}
#define EK_WARN  ::ek::logger::log_stream_t{::ek::logger::verbosity_t::warning, EK_CURRENT_LOCATION}
#define EK_ERROR ::ek::logger::log_stream_t{::ek::logger::verbosity_t::error,   EK_CURRENT_LOCATION}

}


