#include "Camera3D.hpp"
#include "StaticMesh.hpp"
#include "Transform3D.hpp"
#include "Light3D.hpp"
#include "RenderSystem3D.hpp"

#include <ek/graphics/graphics.hpp>
#include "render3d_shader.h"

#include <ek/timers.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/app.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/math/matrix_transform.hpp>
#include <ek/math/matrix_transpose.hpp>
#include <ek/math/matrix_inverse.hpp>
#include <ek/math/matrix_camera.hpp>

#include <cstring>

namespace ek {

const auto DEFAULT_FACE_WINDING = SG_FACEWINDING_CCW;
using namespace graphics;

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

sg_layout_desc getVertex3DLayout() {
    sg_layout_desc layout{};
    layout.buffers[0].stride = sizeof(ModelVertex3D);
    layout.attrs[0].offset = offsetof(ModelVertex3D, position);
    layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT3;
    layout.attrs[1].offset = offsetof(ModelVertex3D, normal);
    layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT3;
    layout.attrs[2].offset = offsetof(ModelVertex3D, uv);
    layout.attrs[2].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[3].offset = offsetof(ModelVertex3D, color);
    layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    layout.attrs[4].offset = offsetof(ModelVertex3D, color2);
    layout.attrs[4].format = SG_VERTEXFORMAT_UBYTE4N;
    return layout;
}

sg_image_desc renderTargetDesc(int w, int h) {
    return sg_image_desc{
            .type = SG_IMAGETYPE_2D,
            .render_target = true,
            .width = w,
            .height = h,
            .usage = SG_USAGE_IMMUTABLE,
            .pixel_format = SG_PIXELFORMAT_RGBA8,
            .sample_count = 1,
            .min_filter = SG_FILTER_LINEAR,
            .mag_filter = SG_FILTER_LINEAR,
            .wrap_u = SG_WRAP_CLAMP_TO_EDGE,
            .wrap_v = SG_WRAP_CLAMP_TO_EDGE,
    };
}

struct ShadowMapRes {
    graphics::Texture* rt = nullptr;
    graphics::Texture* rtColor = nullptr;
    graphics::Shader* shader = nullptr;
    sg_pass pass{};
    sg_pass_action clear{};
    sg_pipeline pip{};

    mat4f projection{};
    mat4f view{};

    void init() {

        shader = new Shader(render3d_shadow_map_shader_desc(sg_query_backend()));
        const uint32_t w = 2048;
        const uint32_t h = 2048;
        auto depthImageDesc = renderTargetDesc(w, h);
        depthImageDesc.pixel_format = SG_PIXELFORMAT_DEPTH;
        depthImageDesc.label = "shadows_depth";
        rt = new Texture(depthImageDesc);
        auto depthColorDesc = renderTargetDesc(w, h);
        depthColorDesc.label = "shadows_tex";
        rtColor = new Texture(depthColorDesc);
        sg_pipeline_desc pipDesc{};
        pipDesc.shader = shader->shader;
        pipDesc.index_type = SG_INDEXTYPE_UINT16;
        pipDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
        pipDesc.cull_mode = SG_CULLMODE_FRONT;
        pipDesc.face_winding = DEFAULT_FACE_WINDING;
        pipDesc.sample_count = 1;
        pipDesc.depth.write_enabled = true;
        pipDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        pipDesc.depth.pixel_format = SG_PIXELFORMAT_DEPTH;
        pipDesc.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;
        pipDesc.layout = getVertex3DLayout();
        pipDesc.label = "3d-shadow-map";
        pip = sg_make_pipeline(pipDesc);

        clear.colors[0].action = SG_ACTION_CLEAR;
        clear.colors[0].value = {1.0f, 1.0f, 1.0f, 1.0f};
        clear.depth.action = SG_ACTION_CLEAR;
        clear.depth.value = 1.0f;

        sg_pass_desc passDesc{};
        passDesc.color_attachments[0].image = rtColor->image;
        passDesc.depth_stencil_attachment.image = rt->image;
        passDesc.label = "shadow-map-pass";
        pass = sg_make_pass(passDesc);
    }

    void begin() {
        auto w = rt->desc.width;
        auto h = rt->desc.height;
        sg_begin_pass(pass, clear);
        sg_apply_viewport(0, 0, w, h, true);
        sg_apply_scissor_rect(0, 0, w, h, true);
        sg_apply_pipeline(pip);
    }

    void updateLightDirection(const mat4f& cameraProjection, const mat4f& cameraView) {
        // find directional light
        float3 light_position{0, 0, 1};
        Light3D light_data{};
        for (auto e : ecs::view<Light3D, Transform3D>()) {
            auto& l = e.get<Light3D>();
            auto& transform = e.get<Transform3D>();
            if (l.type == Light3DType::Directional) {
                light_data = l;
                light_position = normalize(extract_translation(transform.world));
            }
        }

        const float3 light_target = float3::zero;
//    auto light_dir = normalize(light_target - light_position);

        auto bb = get_shadow_map_box(cameraProjection, cameraView, view);
        const float shadow_zone_size = 200.0f;
        view = look_at_rh(light_position, light_target, float3{0, 0, 1});
        projection = ortho_projection_rh<float>(-shadow_zone_size,
                                                shadow_zone_size,
                                                shadow_zone_size,
                                                -shadow_zone_size,
                                                -shadow_zone_size, shadow_zone_size);
    }

    void renderObjects() {
        sg_bindings bindings{};
        mat4f mvp;

        Res<StaticMesh> resMesh{};
        for (auto e: ecs::view<MeshRenderer, Transform3D>()) {
            const auto& filter = e.get<MeshRenderer>();
            if (filter.castShadows && e.get_or_default<Node>().visible()) {
                resMesh.setID(filter.mesh);
                auto* mesh = resMesh.get_or(filter.meshPtr);
                if (mesh) {
                    bindings.index_buffer = mesh->ib.buffer;
                    bindings.vertex_buffers[0] = mesh->vb.buffer;
                    sg_apply_bindings(bindings);
                    mvp = projection * view * e.get<Transform3D>().world;
                    sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(mvp));
                    sg_draw(0, mesh->indices_count, 1);
                }
            }
        }
    }

    void end() {
        sg_end_pass();
    }
};

struct Main3DRes {
    Shader* shader = nullptr;
    sg_pipeline pip{};
    sg_bindings bind{};

    light_params_t directionalLightParams{};
    light2_params_t pointLightParams{};

    void init() {
        shader = new Shader(render3d_shader_desc(sg_query_backend()));

        sg_pipeline_desc pipDesc{};
        pipDesc.label = "3d-main";
        pipDesc.shader = shader->shader;
        pipDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
        pipDesc.index_type = SG_INDEXTYPE_UINT16;
        pipDesc.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        pipDesc.depth.write_enabled = true;
        pipDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        pipDesc.cull_mode = SG_CULLMODE_BACK;
        pipDesc.face_winding = DEFAULT_FACE_WINDING;
        pipDesc.sample_count = 1;
        pipDesc.layout = getVertex3DLayout();

        pip = sg_make_pipeline(pipDesc);
    }

    void setDirectionalLightInfo(float3 pos, const Light3D& data) {
        memcpy(directionalLightParams.light_position, pos.data(), sizeof(float) * 3);
        memcpy(directionalLightParams.light_ambient, data.ambient.data(), sizeof(float) * 3);
        memcpy(directionalLightParams.light_diffuse, data.diffuse.data(), sizeof(float) * 3);
        memcpy(directionalLightParams.light_specular, data.specular.data(), sizeof(float) * 3);
    }

    void setPointLightInfo(float3 pos, const Light3D& data) {
        memcpy(pointLightParams.light2_position, pos.data(), sizeof(float) * 3);
        memcpy(pointLightParams.light2_ambient, data.ambient.data(), sizeof(float) * 3);
        memcpy(pointLightParams.light2_diffuse, data.diffuse.data(), sizeof(float) * 3);
        memcpy(pointLightParams.light2_specular, data.specular.data(), sizeof(float) * 3);
        pointLightParams.light2_radius = data.radius;
        pointLightParams.light2_falloff = data.falloff;
    }
};

struct RenderSkyBoxRes {
    Shader* shader = nullptr;
    sg_pipeline pip{};

    void init() {
        shader = new Shader(render3d_skybox_shader_desc(sg_query_backend()));

        sg_pipeline_desc pipDesc{};
        pipDesc.shader = shader->shader;
        pipDesc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
        pipDesc.index_type = SG_INDEXTYPE_UINT16;
        pipDesc.label = "3d-skybox";
        pipDesc.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
        pipDesc.depth.write_enabled = false;
        pipDesc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        pipDesc.cull_mode = SG_CULLMODE_FRONT;
        pipDesc.face_winding = SG_FACEWINDING_CCW;
        pipDesc.sample_count = 1;
        pipDesc.layout = getVertex3DLayout();

        pip = sg_make_pipeline(pipDesc);
    }

    void render(const Texture* cubeMapTexture, const mat4f& view, const mat4f& projection) {
        Res<StaticMesh> mesh{"cube"};
        if (cubeMapTexture && mesh) {
            sg_apply_pipeline(pip);

            mat4f model{};

            mat4f view3 = view;
            view3.m03 = 0;
            view3.m13 = 0;
            view3.m23 = 0;
            view3.m33 = 1;
            view3.m30 = 0;
            view3.m31 = 0;
            view3.m32 = 0;

            const mat4f mvp = projection * view3 * model;

            sg_bindings bind{};
            bind.fs_images[0] = cubeMapTexture->image;
            bind.vertex_buffers[0] = mesh->vb.buffer;
            bind.index_buffer = mesh->ib.buffer;
            sg_apply_bindings(bind);

            sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(mvp));
            sg_draw(0, mesh->indices_count, 1);
        }
    }
};

RenderSystem3D::RenderSystem3D() {
    shadows = new ShadowMapRes();
    shadows->init();
    main = new Main3DRes();
    main->init();
    skybox = new RenderSkyBoxRes();
    skybox->init();
}

RenderSystem3D::~RenderSystem3D() {
    delete shadows;
    delete main;
    delete skybox;
}

void RenderSystem3D::renderObjects(const mat4f& proj, const mat4f& view) {
    Res<Texture> texEmpty{"empty"};
    main->bind.fs_images[SLOT_uImage0] = texEmpty->image;
    main->bind.fs_images[SLOT_u_image_shadow_map] = shadows->rtColor->image;

    for (auto e: ecs::view<MeshRenderer, Transform3D>()) {
        const auto& filter = e.get<MeshRenderer>();
        auto* mesh = Res<StaticMesh>{filter.mesh}.get_or(filter.meshPtr);
        if (mesh && e.get_or_default<Node>().visible()) {
            mat4f model = e.get<Transform3D>().world;
            mat3f nm = transpose(inverse(mat3f{model}));
            mat4f nm4{};
            nm4.m00 = nm.m00;
            nm4.m01 = nm.m01;
            nm4.m02 = nm.m02;
            nm4.m10 = nm.m10;
            nm4.m11 = nm.m11;
            nm4.m12 = nm.m12;
            nm4.m20 = nm.m20;
            nm4.m21 = nm.m21;
            nm4.m22 = nm.m22;

            const mat4f depth_mvp = shadows->projection * shadows->view * model;

            const auto& material = *(Res<Material3D>{filter.material}.get_or(&defaultMaterial));
            vs_params_t params;
            memcpy(params.uModelViewProjection, (proj * view * model).data_, sizeof(float) * 16);
            memcpy(params.uModel, model.data_, sizeof(float) * 16);
            memcpy(params.u_depth_mvp, depth_mvp.data_, sizeof(float) * 16);
            memcpy(params.u_normal_matrix, nm4.data_, sizeof(float) * 16);
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(params));

            material_params_t matParams;
            memcpy(matParams.mat_diffuse, material.diffuse.data(), sizeof(float) * 3);
            memcpy(matParams.mat_ambient, material.ambient.data(), sizeof(float) * 3);
            memcpy(matParams.mat_specular, material.specular.data(), sizeof(float) * 3);
            memcpy(matParams.mat_emission, material.emission.data(), sizeof(float) * 3);
            matParams.mat_shininess = (1.f / material.roughness) - 1.f;
            matParams.mat_roughness = material.roughness;
            sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_material_params, SG_RANGE(matParams));

            main->bind.index_buffer = mesh->ib.buffer;
            main->bind.vertex_buffers[0] = mesh->vb.buffer;
            sg_apply_bindings(main->bind);
            sg_draw(0, mesh->indices_count, 1);
        }
    }
}

void RenderSystem3D::prepare() {
    defaultMaterial.set_base_color(0xFF00FF_rgb, 0.2f);

    if (!camera.isAlive() || !scene.isAlive()) {
        camera = nullptr;
        scene = nullptr;
    }
    // get view camera orientation
    if (!camera || !isVisible(camera)) {
        return;
    }

    const auto& cameraData = camera.get<Camera3D>();
    const auto& cameraTransform = camera.get<Transform3D>();

    float3 point_light_pos{0, 15, 0};
    Light3D point_light{};

    float3 directional_light_pos{0, 0, -1};
    Light3D directional_light{};
    for (auto e : ecs::view<Light3D, Transform3D>()) {
        auto& l = e.get<Light3D>();
        auto& transform = e.get<Transform3D>();
        if (l.type == Light3DType::Point) {
            point_light = l;
            point_light_pos = extract_translation(transform.world);
        } else if (l.type == Light3DType::Directional) {
            directional_light = l;
            directional_light_pos = normalize(extract_translation(transform.world));
        }
    }

    mat4f view = inverse(cameraTransform.world);

    const float width = app::g_app.drawableWidth;
    const float height = app::g_app.drawableHeight;
    mat4f proj{};
    const auto aspect = (float) width / height;
    if (cameraData.orthogonal) {
        const auto ortho_size = cameraData.orthogonalSize;
        proj = ortho_projection_rh(-ortho_size * aspect, ortho_size * aspect,
                                   -ortho_size, ortho_size,
                                   cameraData.zNear,
                                   cameraData.zFar);
    } else {
        proj = perspective_rh(cameraData.fov, aspect, cameraData.zNear, cameraData.zFar);
    }
    cameraProjection = proj;
    cameraView = view;
    main->setDirectionalLightInfo(directional_light_pos, directional_light);
    main->setPointLightInfo(point_light_pos, point_light);
}

void RenderSystem3D::prerender() {
    sg_push_debug_group("3D shadows");
    shadows->begin();
    shadows->updateLightDirection(cameraProjection, cameraView);
    shadows->renderObjects();
    shadows->end();
    sg_pop_debug_group();
}

void RenderSystem3D::render(float width, float height) {
    /////
    // get view camera orientation
    if (!camera || !isVisible(camera)) {
        return;
    }

    const auto& cameraData = camera.get<Camera3D>();
    const auto& cameraTransform = camera.get<Transform3D>();

    const auto wi = static_cast<int>(width);
    const auto hi = static_cast<int>(height);
    sg_apply_scissor_rect(0, 0, wi, hi, true);
    sg_apply_viewport(0, 0, wi, hi, true);

    static float fc_ = 1.0;
    fc_ += 1.0f;
    auto time = static_cast<float>(Clock::now());

    sg_apply_pipeline(main->pip);

    fs_params_t fs_params;
    fs_params.u_time[0] = time;
    fs_params.u_time[1] = math::fract(time);
    fs_params.u_time[2] = fc_;
    fs_params.u_time[3] = 0.0f;
    fs_params.u_resolution[0] = width;
    fs_params.u_resolution[1] = height;
    fs_params.u_resolution[2] = 1.0f / width;
    fs_params.u_resolution[3] = 1.0f / height;
    auto viewPos = extract_translation(cameraTransform.world);
    memcpy(fs_params.uViewPos, viewPos.data(), sizeof(float) * 3);

    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, SG_RANGE(fs_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light_params, SG_RANGE(main->directionalLightParams));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light2_params, SG_RANGE(main->pointLightParams));

    renderObjects(cameraProjection, cameraView);
    skybox->render(cameraData.cubeMap.get(), cameraView, cameraProjection);
}

}
