#include "obj_loader.hpp"

#include <ek/math/packed_color.hpp>

#include <vector>
#include <sstream>

// strtok
#include <cstring>
// strtol, strtod
#include <cstdlib>

using std::istream;
using std::string;
using std::stringstream;

namespace ek {

struct obj_model_t {

    std::vector<float3> positions;
    std::vector<float3> normals;
    std::vector<float2> uvs;
    std::vector<int> faces;

    void load(std::istream& inOBJ) {
        char buf[256];

        // Read OBJ file
        while (!inOBJ.eof()) {
            string line;
            getline(inOBJ, line);
            string type = line.substr(0, 2);
            size_t line_size = line.size();

            if (type == "v ") {
                memcpy(buf, line.c_str(), line_size);
                buf[line_size] = '\0';
                strtok(buf, " ");
                double x = strtod(strtok(nullptr, " "), nullptr);
                double y = strtod(strtok(nullptr, " "), nullptr);
                double z = strtod(strtok(nullptr, " "), nullptr);
                positions.emplace_back(x, y, z);
            } else if (type == "vt") {
                memcpy(buf, line.c_str(), line_size);
                buf[line_size] = '\0';
                strtok(buf, " ");

                double x = strtod(strtok(nullptr, " "), nullptr);
                double y = strtod(strtok(nullptr, " "), nullptr);
                uvs.emplace_back(x, y);
            } else if (type == "vn") {
                memcpy(buf, line.c_str(), line_size);
                buf[line_size] = '\0';
                strtok(buf, " ");
                double x = strtod(strtok(nullptr, " "), nullptr);
                double y = strtod(strtok(nullptr, " "), nullptr);
                double z = strtod(strtok(nullptr, " "), nullptr);
                normals.emplace_back(x, y, z);
            } else if (type == "f ") {
                memcpy(buf, line.c_str(), line_size);
                buf[line_size] = '\0';
                strtok(buf, " ");
                long p1 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long t1 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long n1 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long p2 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long t2 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long n2 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long p3 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long t3 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                long n3 = strtol(strtok(nullptr, " /"), nullptr, 0) - 1;
                faces.emplace_back(p1);
                faces.emplace_back(t1);
                faces.emplace_back(n1);
                faces.emplace_back(p2);
                faces.emplace_back(t2);
                faces.emplace_back(n2);
                faces.emplace_back(p3);
                faces.emplace_back(t3);
                faces.emplace_back(n3);
            }
        }
    }
};

Model3D load_obj(const std::vector<uint8_t>& buffer) {

    obj_model_t obj_model{};
    {
        const std::string str{
                reinterpret_cast<const char*>(buffer.data()),
                buffer.size()
        };
        stringstream str_stream{str};
        obj_model.load(str_stream);
    }

    Model3D result;
    auto l = obj_model.faces.size();
    const int attributes_per_vertex = 3;
    auto vertices = l / attributes_per_vertex;
    result.vertices.resize(vertices);
    result.indices.resize(vertices);
    size_t ptr = 0;
    for (size_t i = 0; i < vertices; ++i) {
        result.vertices[i] = ModelVertex3D{
                obj_model.positions[obj_model.faces[ptr]],
                obj_model.normals[obj_model.faces[ptr + 2]],
                obj_model.uvs[obj_model.faces[ptr + 1]],
                0xFFFFFFFF,
                0x0
        };
        result.indices[i] = i;
        ptr += attributes_per_vertex;
    }
    return result;
}

}