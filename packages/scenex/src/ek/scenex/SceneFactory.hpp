#pragma once

#include <ek/math.h>
#include <ek/hash.h>
#include <ecxx/ecxx_fwd.hpp>

namespace ek {

class SGFile;
struct SGNodeData;

SGFile* sg_load(const void* data, uint32_t size);

[[nodiscard]]
const SGNodeData* sg_get(const SGFile& sg, string_hash_t libraryName);

ecs::EntityApi sg_create(string_hash_t library, string_hash_t name, ecs::EntityApi parent = nullptr);

rect_t sg_get_bounds(string_hash_t library, string_hash_t name);

ecs::EntityApi createNode2D(string_hash_t tag = 0);
ecs::EntityApi createNode2D(ecs::EntityApi parent, string_hash_t tag = 0, int index = -1);

}


