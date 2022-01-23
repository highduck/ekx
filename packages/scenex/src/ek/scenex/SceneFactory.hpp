#pragma once

#include <ek/math.h>
#include <ek/hash.h>
#include <ek/rr.h>
#include <ek/format/SGFile.hpp>
#include <ecxx/ecxx_fwd.hpp>

namespace ek {

void sg_load(SGFile* out, const void* data, uint32_t size);

[[nodiscard]]
const SGNodeData* sg_get(const SGFile* sg, string_hash_t libraryName);

ecs::EntityApi sg_create(string_hash_t library, string_hash_t name, ecs::EntityApi parent = nullptr);

rect_t sg_get_bounds(string_hash_t library, string_hash_t name);

ecs::EntityApi createNode2D(string_hash_t tag = 0);
ecs::EntityApi createNode2D(ecs::EntityApi parent, string_hash_t tag = 0, int index = -1);

}

struct res_sg {
    string_hash_t names[16];
    ek::SGFile data[16];
    rr_man_t rr;
};

extern struct res_sg res_sg;

void setup_res_sg(void);

#define R_SG(name) REF_NAME(res_sg, name)


