#define FAST_OBJ_IMPLEMENTATION

#include "fast_obj.h"
#include <ek/log.h>
#include <ek/math.h>

typedef struct {
    vec3_t position;
    vec3_t normal;
    vec2_t uv;
    color_t color;
    color_t color2;
} vertex3d_t;

int convertObjModel(const char* input, const char* output) {
    fastObjMesh* mesh = fast_obj_read(input);
    if (!mesh) {
        return 1;
    }

    unsigned int verticesCount = 0;
    unsigned int indicesCount = 0;
    for (int i = 0; i < mesh->face_count; ++i) {
        unsigned int faceVerticesCount = mesh->face_vertices[i];
        verticesCount += faceVerticesCount;
        indicesCount += faceVerticesCount;
    }

    if (indicesCount > 0 && verticesCount > 0) {
        vertex3d_t* vertices = (vertex3d_t*) malloc(sizeof(vertex3d_t) * verticesCount);
        uint16_t* indices = (uint16_t*) malloc(sizeof(uint16_t) * indicesCount);
        vertex3d_t* pVertex = vertices;
        uint16_t* pIndex = indices;

        int vi = 0;
        for (int i = 0; i < mesh->face_count; ++i) {
            const uint32_t faceVerticesCount = mesh->face_vertices[i];
            for (int j = 0; j < faceVerticesCount; ++j) {
                const fastObjIndex face = mesh->indices[vi];
                const float* p = mesh->positions + face.p * 3;
                const float* n = mesh->normals + face.n * 3;
                const float* t = mesh->texcoords + face.t * 2;

                pVertex->position.x = p[0];
                pVertex->position.y = p[1];
                pVertex->position.z = p[2];
                pVertex->normal.x = n[0];
                pVertex->normal.y = n[1];
                pVertex->normal.z = n[2];
                pVertex->uv.x = t[0];
                pVertex->uv.y = t[1];
                pVertex->color = COLOR_WHITE;
                pVertex->color2 = COLOR_ZERO;
                ++pVertex;

                *pIndex = vi;
                ++pIndex;

                ++vi;
            }
        }

        fast_obj_destroy(mesh);

        FILE* f = fopen(output, "wb");
        fwrite(&verticesCount, 1, sizeof verticesCount, f);
        fwrite(vertices, 1, sizeof(vertex3d_t) * verticesCount, f);
        fwrite(&indicesCount, 1, sizeof indicesCount, f);
        fwrite(indices, 1, sizeof(uint16_t) * indicesCount, f);
        fclose(f);

        free(vertices);
        free(indices);
        return 0;
    }

    fast_obj_destroy(mesh);
    return 1;
}

int main(int argc, char** argv) {
    log_init();
    if (argc < 2) {
        return 1;
    }
    const char* command = argv[1];
    log_info("run command %s", command);
    if (!strcmp("obj", command)) {
        if (argc < 4) {
            return 1;
        }
        const char* input = argv[2];
        const char* output = argv[3];
        convertObjModel(input, output);
    }

    return 0;
}