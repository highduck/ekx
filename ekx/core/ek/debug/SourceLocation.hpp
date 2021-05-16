#pragma once

#include "../config.hpp"

namespace ek {

struct SourceLocation {
    const char* file = nullptr;
    unsigned line = 0;
};

#ifdef EK_SOURCE_LOCATION_ENABLED

#define EK_CURRENT_LOCATION ek::SourceLocation{__FILE__, __LINE__}

#else

#define EK_CURRENT_LOCATION ek::SourceLocation{}

#endif

}