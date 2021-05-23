#pragma once

#include <cstring>
#include "Arguments.hpp"

namespace ek {

Arguments Arguments::current = {0, nullptr};

const char* Arguments::getValue(const char* key, const char* def) const {
    for (int i = 0; i < argc; ++i) {
        if (strcmp(key, argv[i]) == 0) {
            if (i + 1 < argc) {
                return argv[i + 1];
            }
            return def;
        }
    }
    return def;
}

}
