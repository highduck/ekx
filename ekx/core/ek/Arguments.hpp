#pragma once

namespace ek {

struct Arguments final {
    static Arguments current;

    int argc = 0;
    char** argv = nullptr;

    const char* getValue(const char* key, const char* def) const;
};

}