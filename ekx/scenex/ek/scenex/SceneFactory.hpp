#pragma once

#include <vector>
#include <string>
#include <ek/math/box.hpp>
#include <ecxx/ecxx_fwd.hpp>

namespace ek {

class SGFile;
struct SGNodeData;

SGFile* sg_load(const std::vector<uint8_t>& buffer);

[[nodiscard]]
const SGNodeData* sg_get(const SGFile& sg, const std::string& libraryName);

ecs::EntityApi sg_create(const std::string& library, const std::string& name);

rect_f sg_get_bounds(const std::string& library, const std::string& name);

ecs::EntityApi createNode2D(const char* name = nullptr);

}


