#pragma once

#include "Verbosity.hpp"
#include "SourceLocation.hpp"

namespace ek {

struct LogMessage {
    Verbosity verbosity = Verbosity::Empty;
    SourceLocation location = {};
    const char* message = nullptr;
    unsigned char frameHash = 0;
};

}