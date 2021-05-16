#pragma once

#include <ek/Allocator.hpp>
#include <ek/util/StaticStorage.hpp>

namespace ek {

struct SxMemory {
    ProxyAllocator sx{memory::systemAllocator, "SceneX"};
    ProxyAllocator particles{sx, "Particles"};
    ProxyAllocator trails{sx, "Trails"};
    ProxyAllocator physics{sx, "Physics"};
};

inline StaticStorage<SxMemory> SxMemory{};

}