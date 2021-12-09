#pragma once

#include <ecxx/ecxx_fwd.hpp>
#include <ek/ds/Array.hpp>
#include <ek/util/Type.hpp>

namespace ek {

struct SGFilter;

struct UglyFilter2D {
    bool enabled = true;
    bool processing = false;
    Array<SGFilter> filters;

    bool pass(const ecs::World& w, ecs::EntityIndex e);
};

ECX_TYPE(16, UglyFilter2D);

}


