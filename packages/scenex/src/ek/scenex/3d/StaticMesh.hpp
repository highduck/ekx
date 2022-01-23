#pragma once

#include "Material3D.hpp"
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>
#include <ek/gfx.h>
#include <ek/format/Model3D.hpp>

namespace ek {

struct StaticMesh {
    Model3D origin;
    sg_buffer vb;
    sg_buffer ib;
    int indices_count;

    explicit StaticMesh(const Model3D& model) : origin{model} {
        sg_buffer_desc desc{};
        desc.usage = SG_USAGE_IMMUTABLE;
        desc.type = SG_BUFFERTYPE_VERTEXBUFFER;
        desc.data.ptr = model.vertices.data();
        desc.data.size = model.vertices.size() * sizeof(ModelVertex3D);
        vb = sg_make_buffer(&desc);
        EK_ASSERT(vb.id != 0);

        desc.usage = SG_USAGE_IMMUTABLE;
        desc.type = SG_BUFFERTYPE_INDEXBUFFER;
        //desc.size = model.indices.size() * sizeof(uint16_t);
        desc.data.ptr = model.indices.data();
        desc.data.size = model.indices.size() * sizeof(uint16_t);
        ib = sg_make_buffer(&desc);
        EK_ASSERT(ib.id != 0);

        indices_count = (int)model.indices.size();
    }
};

struct MeshRenderer /* Component */ {
    StaticMesh* meshPtr = nullptr;
    string_hash_t mesh;
    string_hash_t material;
    bool castShadows = true;
    bool receiveShadows = true;
};

}

struct res_mesh3d {
    string_hash_t names[32];
    ek::StaticMesh* data[32];
    rr_man_t rr;
};

extern struct res_mesh3d res_mesh3d;

void setup_res_mesh3d(void);

#define R_MESH3D(name) REF_NAME(res_mesh3d, name)
