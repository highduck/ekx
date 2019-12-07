#pragma once

#include <scenex/config/ecs.h>
#include <ek/util/signals.hpp>

namespace scenex {

struct tween_t {
    float delay = 0.0f;
    float time = 0.0f;
    float duration = 1.0f;
    ek::signal_t<float> advanced;
    bool auto_destroy = true;
};

}