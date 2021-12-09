#pragma once

#include <ek/math/Rect.hpp>
#include <ecxx/ecxx_fwd.hpp>

namespace ek {

class SGFile;
struct SGNodeData;

SGFile* sg_load(const void* data, uint32_t size);

[[nodiscard]]
const SGNodeData* sg_get(const SGFile& sg, const char* libraryName);

ecs::EntityApi sg_create(const char* library, const char* name, ecs::EntityApi parent = nullptr);

Rect2f sg_get_bounds(const char* library, const char* name);

ecs::EntityApi createNode2D(const char* name = nullptr);
ecs::EntityApi createNode2D(ecs::EntityApi parent, const char* name = nullptr, int index = -1);

}


