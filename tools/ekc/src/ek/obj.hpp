#pragma once

#include <ek/math/packed_color.hpp>
#include <ek/system/system.hpp>
#include <ek/scenex/data/Model3D.hpp>

#define FAST_OBJ_IMPLEMENTATION

#include "../fast_obj.h"

namespace ek {

void convertObjModel(const char* input, const char* output) {
    fastObjMesh* mesh = fast_obj_read(input);
    if (!mesh) {
        return;
    }
    Model3D model{};
    int vi = 0;
    for (int i = 0; i < mesh->face_count; ++i) {
        auto verticesCount = mesh->face_vertices[i];
        for (int j = 0; j < verticesCount; ++j) {
            auto face = mesh->indices[vi];
            ModelVertex3D vertex{};
            const float* p = mesh->positions + face.p * 3;
            const float* n = mesh->normals + face.n * 3;
            const float* t = mesh->texcoords + face.t * 2;
            vertex.position.x = p[0];
            vertex.position.y = p[1];
            vertex.position.z = p[2];
            vertex.normal.x = n[0];
            vertex.normal.y = n[1];
            vertex.normal.z = n[2];
            vertex.uv.x = t[0];
            vertex.uv.y = t[1];
            vertex.color = 0xFFFFFFFF;
            vertex.color2 = 0x0;

            model.vertices.emplace_back(vertex);
            model.indices.emplace_back(vi);
            ++vi;
        }
    }
    fast_obj_destroy(mesh);

    output_memory_stream out{100};
    IO io{out};
    io(model);
    ::ek::save(out, output);
}

}