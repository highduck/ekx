#include <ek/math.h>
#include <ek/scenex/data/Model3D.hpp>

#define FAST_OBJ_IMPLEMENTATION

#include "fast_obj.h"

int convertObjModel(const char* input, const char* output) {
    using namespace ek;
    fastObjMesh* mesh = fast_obj_read(input);
    if (!mesh) {
        return 1;
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
            vertex.color = rgba_u32(0xFFFFFFFF);
            vertex.color2 = rgba_u32(0x0);

            model.vertices.emplace_back(vertex);
            model.indices.emplace_back(vi);
            ++vi;
        }
    }
    fast_obj_destroy(mesh);

    output_memory_stream out{100};
    IO io{out};
    io(model);

    auto f = fopen(output, "wb");
    fwrite(out.data(), out.size(), 1, f);
    fclose(f);

    return 0;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        return 1;
    }
    const char* input = argv[1];
    const char* output = argv[2];
    convertObjModel(input, output);

    return 0;
}