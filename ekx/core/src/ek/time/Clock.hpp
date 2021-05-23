#pragma once

#include <cstdint>

namespace ek {

struct Clock final {

static void setup();

static double now();

static uint64_t now64();

};

}