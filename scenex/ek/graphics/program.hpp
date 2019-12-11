#pragma once

#include "graphics.hpp"
#include <ek/math/vec.hpp>
#include <ek/math/mat4x4.hpp>
#include <ek/graphics/gl_def.hpp>
#include <string>
#include <unordered_map>

namespace ek {

struct vertex_decl_t;

namespace program_attributes {
static std::string position = "aPosition";
static std::string normal = "aNormal";
static std::string tex_coord = "aTexCoord";
static std::string color_multiplier = "aColorMult";
static std::string color_offset = "aColorOffset";
}

namespace program_uniforms {
static std::string mvp = "uModelViewProjection";
static std::string depth_mvp = "u_depth_mvp";
static std::string model_matrix = "uModel";
static std::string normal_matrix = "u_normal_matrix";
static std::string view_position = "uViewPos";

static std::string frame_time = "u_time";
static std::string frame_resolution = "u_resolution";

static std::string offset_2d = "uOffset";
static std::string image_0 = "uImage0";
static std::string image_shadow_map = "u_image_shadow_map";

//// light props
//static std::string light_position = "u_light_position";
//static std::string light_ambient = "u_light_ambient";
//static std::string light_diffuse = "u_light_diffuse";
//static std::string light_specular = "u_light_specular";
//
//static std::string point_light_radius = "u_point_light_radius";
//static std::string point_light_falloff = "u_point_light_falloff";
//
//// directional lighting
//static std::string light0_position = "u_light0_position";
//static std::string light0_ambient = "u_light0_ambient";
//static std::string light0_diffuse = "u_light0_diffuse";
//static std::string light0_specular = "u_light0_specular";

// material uniforms
static std::string material_ambient = "u_material_ambient";
static std::string material_diffuse = "u_material_diffuse";
static std::string material_specular = "u_material_specular";
static std::string material_shininess = "u_material_shininess";
static std::string material_emission = "u_material_emission";

}

class program_t : private disable_copy_assign_t {
public:
    vertex_decl_t* vertex = nullptr;

    GLint a_position;
    GLint a_normal;
    GLint a_texcoord;
    GLint a_color_multiplier;
    GLint a_color_offset;

    GLint u_image0_unit = 0;

    program_t(const char* vs_code, const char* fs_code);

    ~program_t();

    void bind_attributes() const;

    void unbind_attributes() const;

    void bind_image() const;

    void use() const;

    inline GLuint handle() const {
        return handle_;
    }

    GLint get_uniform(const std::string& name) const;

    void set_uniform(const std::string& name, const float3& v) const;

    void set_uniform(const std::string& name, const float4& v) const;

    void set_uniform(const std::string& name, const float2& v) const;

    void set_uniform(const std::string& name, const mat4f& v) const;

    void set_uniform(const std::string& name, const mat3f& v) const;

    void set_uniform(const std::string& name, float v) const;

    void set_uniform(const std::string& name, int v) const;

private:
    GLuint handle_;
    mutable std::unordered_map<std::string, GLint> uniforms_cache_;

};

}