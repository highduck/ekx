#pragma once

#include <ek/math/packed_color.hpp>
#include <ek/math/vec.hpp>

namespace ek::graphics {

struct vertex_decl_t {
    int size;
    int position_components;
    bool normals;
};

struct vertex_minimal_2d {

    float2 position;
    float2 uv;
    premultiplied_abgr32_t cm;

    static vertex_decl_t decl;
};

struct vertex_2d {

    float2 position;
    float2 uv;
    premultiplied_abgr32_t cm;
    abgr32_t co;

    static vertex_decl_t decl;
};

struct vertex_3d {
    float3 position;
    float3 normal;
    float2 uv;
    premultiplied_abgr32_t color;
    abgr32_t color2;

    static vertex_decl_t decl;
};

}