#pragma once

#include "world.hpp"

namespace ecxx {

struct wsd_ {
    static world* ptr;
};

inline world* wsd_::ptr{};

//inline static world* wsd_{};

inline void set_world(world* w) {
    wsd_::ptr = w;
}

inline world& get_world() {
    return *wsd_::ptr;
}

}