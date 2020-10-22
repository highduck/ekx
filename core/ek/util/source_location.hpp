#pragma once

namespace ek {

struct source_location_t {
    const char* file = nullptr;
    unsigned line = 0;

    friend std::ostream& operator<<(std::ostream& out, const source_location_t& location) {
        if (location.file) {
            out << location.file;
            if (location.line) {
                out << ':' << location.line;
            }
        }
        return out;
    }
};

#ifdef NDEBUG
#define EK_CURRENT_LOCATION ::ek::source_location_t{}
#else
#define EK_CURRENT_LOCATION ::ek::source_location_t{__FILE__, __LINE__}
#endif

}