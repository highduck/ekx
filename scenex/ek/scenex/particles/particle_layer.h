#pragma once

#include <vector>
#include <ek/scenex/particles/particle.h>

namespace ek {

struct particle_layer_t {
    std::vector<particle*> particles;
    bool cycled = false;
    bool keep_alive = false;
};

}