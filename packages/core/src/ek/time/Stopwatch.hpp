#pragma once

#include <cstdint>

namespace ek {

class Stopwatch final {
public:
    Stopwatch();

    [[nodiscard]]
    float readSeconds() const;

    [[nodiscard]]
    float readMillis() const;

    void reset();

private:
    uint64_t initial_;
};

}