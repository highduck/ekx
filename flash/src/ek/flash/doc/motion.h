#pragma once

namespace ek::flash {

struct motion_object_t {
    int duration;
    int timeScale;
};

enum class tween_type {
    none,
    motion
};

enum class rotation_direction {
    none,
    ccw,
    cw
};

}