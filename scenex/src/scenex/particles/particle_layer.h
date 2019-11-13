#pragma once

#include <vector>
#include <scenex/particles/particle.h>

namespace scenex {

struct particle_layer_t {
    std::vector<particle*> particles;
    bool cycled = false;
    bool keep_alive = false;
};

}