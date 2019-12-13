#include "camera_3d.hpp"
#include "static_mesh.hpp"
#include "transform_3d.hpp"
#include "light_3d.hpp"

#include <ek/util/timer.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/math/matrix_transform.hpp>
#include <ek/math/matrix_transpose.hpp>
#include <ek/math/matrix_inverse.hpp>
#include <ek/math/matrix_camera.hpp>

#include <ek/graphics/render_target.hpp>
#include <ek/graphics/gl_debug.hpp>
#include <ek/scenex/utility/scene_management.hpp>

//#undef near
//#undef far

namespace ek {

using namespace graphics;

static material_3d default_material_{};
static render_target_t* shadow_map_ = nullptr;

void render_objects_to_shadow_map(const mat4f& proj, const mat4f& view) {
    asset_t<program_t> program3d{"3d_shadow_map"};

    for (auto e: ecs::view<mesh_renderer_component, transform_3d>()) {
        const auto& filter = ecs::get<mesh_renderer_component>(e);
        if (filter.cast_shadows) {
            auto* mesh = asset_t<static_mesh_t>{filter.mesh}.get_or(filter.mesh_ptr);
            if (mesh) {
                mat4f model{ecs::get<transform_3d>(e).world};
                draw2d::state.set_mvp(proj * view * model);
                draw2d::invalidate_force();
                draw2d::draw_mesh(mesh->vb, mesh->ib, mesh->indices_count);
            }
        }
    }
}

static mat4f depth_projection_;
static mat4f depth_view_;

box_t<3, float> get_shadow_map_box(const mat4f& camera_projection, const mat4f& camera_view, const mat4f& light_view) {
    const mat4f inv_proj_view = inverse(camera_projection * camera_view);
    float3 corners[8] = {
            float3{-1, -1, -1},
            float3{-1, -1, 1},
            float3{1, -1, -1},
            float3{1, -1, 1},
            float3{-1, 1, -1},
            float3{-1, 1, 1},
            float3{1, 1, -1},
            float3{1, 1, 1}
    };
    float3 bb_min{100000, 100000, 100000};
    float3 bb_max{-100000, -100000, -100000};

    for (size_t i = 0; i < 8; ++i) {
        vec4_t<float> c{corners[i], 1.0f};
        vec4_t<float> v2 = inv_proj_view * c;
        auto len = length(v2);
        vec3_t<float> v = len * normalize(vec3_t<float>{v2.x, v2.y, v2.z});
        if (v.x < bb_min.x) bb_min.x = v.x;
        if (v.y < bb_min.y) bb_min.y = v.y;
        if (v.z < bb_min.z) bb_min.z = v.z;
        if (v.x > bb_max.x) bb_max.x = v.x;
        if (v.y > bb_max.y) bb_max.y = v.y;
        if (v.z > bb_max.z) bb_max.z = v.z;
    }
    return {bb_min, bb_max - bb_min};
}

void render_shadow_map(const mat4f& camera_projection, const mat4f& camera_view) {
    asset_t<program_t> program3d{"3d_shadow_map"};
    if (!program3d) {
        return;
    }
    draw2d::state.save_program()
            .set_program(program3d.get());

    glCullFace(GL_FRONT);
    gl::check_error();

    const uint32_t shadow_map_width = 2048;
    const uint32_t shadow_map_height = 2048;
    if (shadow_map_ == nullptr) {
        shadow_map_ = new render_target_t(shadow_map_width, shadow_map_height, texture_type::depth24);
    }
    graphics::viewport(0, 0, shadow_map_width, shadow_map_height);
    shadow_map_->clear();
    shadow_map_->set();

    // find directional light
    float3 light_position{0, 0, 1};
    light_3d light_data{};
    for (auto e : ecs::view<light_3d, transform_3d>()) {
        auto& l = ecs::get<light_3d>(e);
        auto& transform = ecs::get<transform_3d>(e);
        if (l.type == light_3d_type::directional) {
            light_data = l;
            light_position = normalize(extract_translation(transform.world));
        }
    }

    const float3 light_target = float3::zero;
//    auto light_dir = normalize(light_target - light_position);
    const float shadow_zone_size = 100;
    depth_view_ = look_at_rh(light_position, light_target, float3{0, 0, 1});

    auto bb = get_shadow_map_box(camera_projection, camera_view, depth_view_);
    depth_projection_ = ortho_projection_rh<float>(-shadow_zone_size,
                                                   shadow_zone_size,
                                                   -shadow_zone_size,
                                                   shadow_zone_size,
                                                   -shadow_zone_size,
                                                   2 * shadow_zone_size);
    render_objects_to_shadow_map(depth_projection_, depth_view_);

    shadow_map_->unset();
    draw2d::state.restore_program();
    graphics::viewport();
}

bool begin_3d() {
    asset_t<program_t> program3d{"3d"};
    if (!program3d) {
        return false;
    }

    draw2d::state
            .save_program()
            .save_texture()
            .save_mvp()
            .set_empty_texture()
            .set_program(program3d.get());

    draw2d::commit_state();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL); // equal needs to correct pass skybox z = 1
    glDepthRange(0.0f, 1.0f);

    gl::check_error();

    return true;
}

void end_3d() {
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    gl::check_error();

    draw2d::state.restore_program();
    draw2d::state.restore_texture();
    draw2d::state.restore_mvp();
}

void invalidate_matrix_3d() {
    for (auto e: ecs::view<transform_3d, node_t>()) {
        auto& tr = ecs::get<transform_3d>(e);
        tr.local = translate_transform(tr.position)
                   * rotation_transform(quat_t<float>(tr.rotation))
                   * scale_transform(tr.scale);
        {
            mat4f world = tr.local;
            auto p = ecs::get<node_t>(e).parent;
            while (p) {
                if (ecs::has<transform_3d>(p)) {
                    world = ecs::get<transform_3d>(p).local * world;
                }
                p = ecs::get<node_t>(p).parent;
            }
            tr.world = world;
        }
    }
}

void render_3d_objects(const mat4f& proj, const mat4f& view) {
    asset_t<program_t> program3d{"3d"};

//    mat4f pv = proj * view;
    for (auto e: ecs::view<mesh_renderer_component, transform_3d>()) {
        const auto& filter = ecs::get<mesh_renderer_component>(e);
        auto* mesh = asset_t<static_mesh_t>{filter.mesh}.get_or(filter.mesh_ptr);
        if (mesh) {
            mat4f model{ecs::get<transform_3d>(e).world};
            draw2d::state.set_mvp(proj * view * model);
            draw2d::invalidate_force();

            program3d->set_uniform(program_uniforms::model_matrix, model);
            program3d->set_uniform(program_uniforms::normal_matrix, transpose(inverse(mat3f{model})));

            const mat4f depth_mvp = depth_projection_ * depth_view_ * model;
            program3d->set_uniform(program_uniforms::depth_mvp, depth_mvp);
            program3d->set_uniform(program_uniforms::image_shadow_map, 1);
            shadow_map_->texture()->bind(1);

            const auto& material = *(asset_t<material_3d>{filter.material}.get_or(&default_material_));
            program3d->set_uniform("u_material.ambient", material.ambient);
            program3d->set_uniform("u_material.diffuse", material.diffuse);
            program3d->set_uniform("u_material.specular", material.specular);
//            program3d->set_uniform("u_material.shininess", material.shininess);
            program3d->set_uniform("u_material.shininess", (1.f / material.roughness) - 1.f);
            program3d->set_uniform("u_material.roughness", material.roughness);
            program3d->set_uniform("u_material.emission", material.emission);

//            program3d->set_uniform(program_uniforms::mvp, proj * view * model);
            draw2d::draw_mesh(mesh->vb, mesh->ib, mesh->indices_count);
        }
    }

    glActiveTexture(GL_TEXTURE0 + (GLenum) 1);
    glBindTexture(GL_TEXTURE_2D, 0);
    gl::check_error();
}

void clear_camera(const camera_3d& camera_data) {
    GLbitfield clear_bits = 0;
    if (camera_data.clear_color_enabled) {
        glClearColor(
                camera_data.clear_color.r,
                camera_data.clear_color.g,
                camera_data.clear_color.b,
                camera_data.clear_color.a
        );
        gl::check_error();
        clear_bits |= GL_COLOR_BUFFER_BIT;
    }
    if (camera_data.clear_depth_enabled) {
        glClearDepth(camera_data.clear_depth);
        gl::check_error();
        clear_bits |= GL_DEPTH_BUFFER_BIT;
    }
    if (clear_bits != 0) {
        glClear(clear_bits);
        gl::check_error();
    }
}

void render_skybox(const std::string& id, const mat4f& view, const mat4f& projection);

void render_3d_scene(ecs::entity scene, ecs::entity camera_entity) {

    default_material_.set_base_color(0xFF00FF_rgb, 0.2f);

    // get view camera orientation
    if (!camera_entity || !is_visible(camera_entity)) {
        return;
    }

    auto& camera_data = ecs::get<camera_3d>(camera_entity);
    auto& camera_transform = ecs::get<transform_3d>(camera_entity);

    invalidate_matrix_3d();

    if (!begin_3d()) {
        return;
    }

    float3 point_light_pos{0, 15, 0};
    light_3d point_light{};

    float3 directional_light_pos{0, 0, -1};
    light_3d directional_light{};
    for (auto e : ecs::view<light_3d, transform_3d>()) {
        auto& l = ecs::get<light_3d>(e);
        auto& transform = ecs::get<transform_3d>(e);
        if (l.type == light_3d_type::point) {
            point_light = l;
            point_light_pos = extract_translation(transform.world);
        } else if (l.type == light_3d_type::directional) {
            directional_light = l;
            directional_light_pos = normalize(extract_translation(transform.world));
        }
    }

    mat4f model{};
    mat4f view = inverse(camera_transform.world);

    const float width = app::g_app.drawable_size.x;
    const float height = app::g_app.drawable_size.y;
    mat4f proj{};
    const auto aspect = (float) width / height;
    if (camera_data.orthogonal) {
        const auto ortho_size = camera_data.orthogonal_size;
        proj = ortho_projection_rh(-ortho_size * aspect, ortho_size * aspect,
                                   -ortho_size, ortho_size,
                                   camera_data.near,
                                   camera_data.far);
    } else {
        proj = perspective_rh(camera_data.fov, aspect, camera_data.near, camera_data.far);
    }

    render_shadow_map(proj, view);
    /////
    glCullFace(GL_BACK);

    clear_camera(camera_data);
    mat4f mvp = proj * view * model;

    draw2d::state.set_mvp(mvp);
    draw2d::invalidate_force();

    asset_t<program_t> program3d{"3d"};
    program3d->set_uniform(program_uniforms::view_position, extract_translation(camera_transform.world));

    program3d->set_uniform("u_lights[0].position", directional_light_pos);
    program3d->set_uniform("u_lights[0].ambient", directional_light.ambient);
    program3d->set_uniform("u_lights[0].diffuse", directional_light.diffuse);
    program3d->set_uniform("u_lights[0].specular", directional_light.specular);

    program3d->set_uniform("u_lights[1].position", point_light_pos);
    program3d->set_uniform("u_lights[1].ambient", point_light.ambient);
    program3d->set_uniform("u_lights[1].diffuse", point_light.diffuse);
    program3d->set_uniform("u_lights[1].specular", point_light.specular);
    program3d->set_uniform("u_lights[1].radius", point_light.radius);
    program3d->set_uniform("u_lights[1].falloff", point_light.falloff);

    static float fc_ = 1.0;
    fc_ += 1.0f;
    auto time = static_cast<float>(clock::now());
    program3d->set_uniform(program_uniforms::frame_time, float4{
            time,
            math::fract(time),
            fc_,
            0.0f
    });

    const float2 res{
            static_cast<float>(app::g_app.drawable_size.x),
            static_cast<float>(app::g_app.drawable_size.y)
    };

    program3d->set_uniform(program_uniforms::frame_resolution, float4{
            res.x,
            res.y,
            1.0f / res.x,
            1.0f / res.y
    });


    render_3d_objects(proj, view);

    render_skybox(camera_data.cube_map, view, proj);

    end_3d();
}

void render_skybox(const std::string& id, const mat4f& view, const mat4f& projection) {
    asset_t<texture_t> texture{id};
    asset_t<program_t> program{"3d_skybox"};
    asset_t<static_mesh_t> mesh{"cube"};
    if (texture && program && mesh) {
        mat4f model{};

        mat4f view3 = view;
        view3.m03 = 0;
        view3.m13 = 0;
        view3.m23 = 0;
        view3.m33 = 1;
        view3.m30 = 0;
        view3.m31 = 0;
        view3.m32 = 0;

        const auto mvp = projection * view3 * model;

        draw2d::state
                .save_texture()
                .save_program()
                .save_mvp()
                .set_texture(texture.get())
                .set_program(program.get())
                .set_mvp(mvp);
        draw2d::commit_state();
        draw2d::invalidate_force();

        program->set_uniform(program_uniforms::mvp, projection * view3 * model);

        glDepthMask(GL_FALSE);
        glCullFace(GL_FRONT);
        draw2d::draw_mesh(mesh->vb, mesh->ib, mesh->indices_count);
        glDepthMask(GL_TRUE);
        glCullFace(GL_BACK);

        draw2d::state
                .restore_texture()
                .restore_program()
                .restore_mvp();
    }
}

}
