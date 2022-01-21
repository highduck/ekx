#include "Camera3D.hpp"
#include "StaticMesh.hpp"
#include "Transform3D.hpp"
#include "Light3D.hpp"
#include "RenderSystem3D.hpp"

#include <ek/gfx.h>
#include "render3d_shader.h"

#include <ek/time.h>
#include <ek/canvas.h>
#include <ek/app.h>
#include <ek/scenex/base/Node.hpp>

#include <cstring>

#undef near
#undef far

namespace ek {

const auto DEFAULT_FACE_WINDING = SG_FACEWINDING_CCW;

aabb3_t
get_shadow_map_box(const mat4_t& camera_projection, const mat4_t& camera_view, const mat4_t& light_view) {
    const mat4_t inv_proj_view = mat4_inverse(mat4_mul(camera_projection, camera_view));
    const vec3_t corners[8] = {
            vec3(-1, -1, -1),
            vec3(-1, -1, 1),
            vec3(1, -1, -1),
            vec3(1, -1, 1),
            vec3(-1, 1, -1),
            vec3(-1, 1, 1),
            vec3(1, 1, -1),
            vec3(1, 1, 1),
    };
    aabb3_t bb;
    bb.min = vec3(100000, 100000, 100000);
    bb.max = vec3(-100000, -100000, -100000);

    for (uint32_t i = 0; i < 8; ++i) {
        vec4_t c;
        c.xyz = corners[i];
        c.w = 1;
        vec4_t v2 = mat4_mul_vec4(inv_proj_view, c);
        auto len = length_vec4(v2);
        vec3_t v = scale_vec3(normalize_vec3(v2.xyz), len);
        if (v.x < bb.min.x) bb.min.x = v.x;
        if (v.y < bb.min.y) bb.min.y = v.y;
        if (v.z < bb.min.z) bb.min.z = v.z;
        if (v.x > bb.max.x) bb.max.x = v.x;
        if (v.y > bb.max.y) bb.max.y = v.y;
        if (v.z > bb.max.z) bb.max.z = v.z;
    }
    return bb;
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
    sg_image rt = {0};
    sg_image rtColor = {0};
    ek_shader shader{};
    sg_pass pass{};
    sg_pass_action clear{};
    sg_pipeline pip{};

    mat4_t projection = mat4_identity();
    mat4_t view = mat4_identity();

    void init() {

        shader = ek_shader_make(render3d_shadow_map_shader_desc(sg_query_backend()));
        const uint32_t w = 2048;
        const uint32_t h = 2048;
        auto depthImageDesc = renderTargetDesc(w, h);
        depthImageDesc.pixel_format = SG_PIXELFORMAT_DEPTH;
        depthImageDesc.label = "shadows_depth";
        rt = sg_make_image(depthImageDesc);
        auto depthColorDesc = renderTargetDesc(w, h);
        depthColorDesc.label = "shadows_tex";
        rtColor = sg_make_image(depthColorDesc);
        sg_pipeline_desc pipDesc{};
        pipDesc.shader = shader.shader;
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
        passDesc.color_attachments[0].image = rtColor;
        passDesc.depth_stencil_attachment.image = rt;
        passDesc.label = "shadow-map-pass";
        pass = sg_make_pass(passDesc);
    }

    void begin() {
        auto info = sg_query_image_info(rt);
        auto w = info.width;
        auto h = info.height;
        sg_begin_pass(pass, clear);
        sg_apply_viewport(0, 0, w, h, true);
        sg_apply_scissor_rect(0, 0, w, h, true);
        sg_apply_pipeline(pip);
    }

    void updateLightDirection(const mat4_t& cameraProjection, const mat4_t& cameraView) {
        // find directional light
        vec3_t light_position = vec3(0, 0, 1);
        Light3D light_data{};
        for (auto e: ecs::view<Light3D, Transform3D>()) {
            auto& l = e.get<Light3D>();
            auto& transform = e.get<Transform3D>();
            if (l.type == Light3DType::Directional) {
                light_data = l;
                light_position = normalize_vec3(mat4_get_position(&transform.world));
            }
        }

        const vec3_t light_target = vec3(0, 0, 0);
//    auto light_dir = normalize(light_target - light_position);

        auto bb = get_shadow_map_box(cameraProjection, cameraView, view);
        const float shadow_zone_size = 200.0f;
        view = mat4_look_at_rh(light_position, light_target, vec3(0, 0, 1));
        projection = mat4_orthographic_rh(-shadow_zone_size,
                                          shadow_zone_size,
                                          shadow_zone_size,
                                          -shadow_zone_size,
                                          -shadow_zone_size,
                                          shadow_zone_size);
    }

    void renderObjects() {
        sg_bindings bindings{};
        mat4_t mvp;

        R(StaticMesh) resMesh = 0;
        for (auto e: ecs::view<MeshRenderer, Transform3D>()) {
            const auto& filter = e.get<MeshRenderer>();
            if (filter.castShadows && e.get_or_default<Node>().visible()) {
                resMesh = R_MESH3D(filter.mesh);
                auto* mesh = resMesh ? REF_RESOLVE(res_mesh3d, resMesh) : nullptr;
                if(!mesh) {
                    mesh = filter.meshPtr;
                }
                if (mesh) {
                    bindings.index_buffer = mesh->ib;
                    bindings.vertex_buffers[0] = mesh->vb;
                    sg_apply_bindings(bindings);
                    mvp = mat4_mul(mat4_mul(projection, view), e.get<Transform3D>().world);
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
    ek_shader shader{};
    sg_pipeline pip{};
    sg_bindings bind{};

    light_params_t directionalLightParams{};
    light2_params_t pointLightParams{};

    void init() {
        shader = ek_shader_make(render3d_shader_desc(sg_query_backend()));

        sg_pipeline_desc pipDesc{};
        pipDesc.label = "3d-main";
        pipDesc.shader = shader.shader;
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

    void setDirectionalLightInfo(vec3_t pos, const Light3D& data) {
        memcpy(directionalLightParams.light_position, &pos, sizeof(vec3_t));
        memcpy(directionalLightParams.light_ambient, &data.ambient, sizeof(vec3_t));
        memcpy(directionalLightParams.light_diffuse, &data.diffuse, sizeof(vec3_t));
        memcpy(directionalLightParams.light_specular, &data.specular, sizeof(vec3_t));
    }

    void setPointLightInfo(vec3_t pos, const Light3D& data) {
        memcpy(pointLightParams.light2_position, pos.data, sizeof(vec3_t));
        memcpy(pointLightParams.light2_ambient, data.ambient.data, sizeof(vec3_t));
        memcpy(pointLightParams.light2_diffuse, data.diffuse.data, sizeof(vec3_t));
        memcpy(pointLightParams.light2_specular, data.specular.data, sizeof(vec3_t));
        pointLightParams.light2_radius = data.radius;
        pointLightParams.light2_falloff = data.falloff;
    }
};

struct RenderSkyBoxRes {
    ek_shader shader{};
    sg_pipeline pip{};

    void init() {
        shader = ek_shader_make(render3d_skybox_shader_desc(sg_query_backend()));

        sg_pipeline_desc pipDesc{};
        pipDesc.shader = shader.shader;
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

    void render(const sg_image cubemap, const mat4_t& view, const mat4_t& projection) {
        StaticMesh* mesh = RES_NAME_RESOLVE(res_mesh3d, H("cube"));
        if (cubemap.id && mesh) {
            sg_apply_pipeline(pip);

            mat4_t model = mat4_identity();

            mat4_t view3 = view;
            view3.m03 = 0;
            view3.m13 = 0;
            view3.m23 = 0;
            view3.m30 = 0;
            view3.m31 = 0;
            view3.m32 = 0;
            view3.m33 = 1;

            const mat4_t mvp = mat4_mul(mat4_mul(projection, view3), model);

            sg_bindings bind{};
            bind.fs_images[0] = cubemap;
            bind.vertex_buffers[0] = mesh->vb;
            bind.index_buffer = mesh->ib;
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

void RenderSystem3D::renderObjects(mat4_t proj, mat4_t view) {
    const sg_image empty = res_image.data[R_IMAGE_EMPTY];
    main->bind.fs_images[SLOT_uImage0] = empty;
    main->bind.fs_images[SLOT_u_image_shadow_map] = shadows->rtColor;

    for (auto e: ecs::view<MeshRenderer, Transform3D>()) {
        const auto& filter = e.get<MeshRenderer>();
        auto* mesh = filter.mesh ? RES_NAME_RESOLVE(res_mesh3d, filter.mesh) : nullptr;
        if(!mesh) mesh = filter.meshPtr;
        if (mesh && e.get_or_default<Node>().visible()) {
            mat4_t model = e.get<Transform3D>().world;
//            mat3_t nm = mat3_transpose(mat3_inverse(mat4_get_mat3(&model)));
//            mat4_t nm4 = mat4_mat3(nm);
            mat4_t nm4 = mat4_transpose(mat4_inverse(mat4_mat3(mat4_get_mat3(&model))));

            const mat4_t depth_mvp = mat4_mul(mat4_mul(shadows->projection, shadows->view), model);

            const auto& material = RES_NAME_RESOLVE(res_material3d, filter.material);
            vs_params_t params;
            memcpy(params.uModelViewProjection, mat4_mul(mat4_mul(proj, view), model).data, sizeof(mat4_t));
            memcpy(params.uModel, model.data, sizeof(mat4_t));
            memcpy(params.u_depth_mvp, depth_mvp.data, sizeof(mat4_t));
            memcpy(params.u_normal_matrix, nm4.data, sizeof(mat4_t));
            sg_apply_uniforms(SG_SHADERSTAGE_VS, SLOT_vs_params, SG_RANGE(params));

            material_params_t matParams;
            memcpy(matParams.mat_diffuse, material.diffuse.data, sizeof(vec3_t));
            memcpy(matParams.mat_ambient, material.ambient.data, sizeof(vec3_t));
            memcpy(matParams.mat_specular, material.specular.data, sizeof(vec3_t));
            memcpy(matParams.mat_emission, material.emission.data, sizeof(vec3_t));
            matParams.mat_shininess = (1.f / material.roughness) - 1.f;
            matParams.mat_roughness = material.roughness;
            sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_material_params, SG_RANGE(matParams));

            main->bind.index_buffer = mesh->ib;
            main->bind.vertex_buffers[0] = mesh->vb;
            sg_apply_bindings(main->bind);
            sg_draw(0, mesh->indices_count, 1);
        }
    }
}

void RenderSystem3D::prepare() {
    defaultMaterial.set_base_color(RGB(0xFF00FF), 0.2f);

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

    vec3_t point_light_pos = vec3(0, 15, 0);
    Light3D point_light{};

    vec3_t directional_light_pos = vec3(0, 0, 1);
    Light3D directional_light{};
    for (auto e: ecs::view<Light3D, Transform3D>()) {
        auto& l = e.get<Light3D>();
        auto& transform = e.get<Transform3D>();
        if (l.type == Light3DType::Point) {
            point_light = l;
            point_light_pos = mat4_get_position(&transform.world);
        } else if (l.type == Light3DType::Directional) {
            directional_light = l;
            directional_light_pos = normalize_vec3(mat4_get_position(&transform.world));
        }
    }

    mat4_t view = mat4_inverse(cameraTransform.world);

    const float width = ek_app.viewport.width;
    const float height = ek_app.viewport.height;
    mat4_t proj;
    const auto aspect = (float) width / height;
    if (cameraData.orthogonal) {
        const auto ortho_size = cameraData.orthogonalSize;
        proj = mat4_orthographic_rh(-ortho_size * aspect,
                                    ortho_size * aspect,
                                    -ortho_size,
                                    ortho_size,
                                    cameraData.zNear,
                                    cameraData.zFar);
    } else {
        proj = mat4_perspective_rh(cameraData.fov, aspect, cameraData.zNear, cameraData.zFar);
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
    auto time = static_cast<float>(ek_time_now());

    sg_apply_pipeline(main->pip);

    fs_params_t fs_params;
    fs_params.u_time[0] = time;
    fs_params.u_time[1] = fract(time);
    fs_params.u_time[2] = fc_;
    fs_params.u_time[3] = 0.0f;
    fs_params.u_resolution[0] = width;
    fs_params.u_resolution[1] = height;
    fs_params.u_resolution[2] = 1.0f / width;
    fs_params.u_resolution[3] = 1.0f / height;
    auto viewPos = mat4_get_position(&cameraTransform.world);
    memcpy(fs_params.uViewPos, viewPos.data, sizeof(float) * 3);

    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_fs_params, SG_RANGE(fs_params));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light_params, SG_RANGE(main->directionalLightParams));
    sg_apply_uniforms(SG_SHADERSTAGE_FS, SLOT_light2_params, SG_RANGE(main->pointLightParams));

    renderObjects(cameraProjection, cameraView);
    skybox->render(REF_RESOLVE(res_image, cameraData.cubeMap), cameraView, cameraProjection);
}

}


//// resources

struct res_material3d res_material3d;
struct res_mesh3d res_mesh3d;

void setup_res_material3d(void) {
    struct res_material3d* R = &res_material3d;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

void setup_res_mesh3d(void) {
    struct res_mesh3d* R = &res_mesh3d;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}
