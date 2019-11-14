#include "main.hpp"
#include "camera_arcball.hpp"

#include <scenex/systems/main_flow.h>
#include <sim/driver_node_script.h>
#include <scenex/scene_system.h>
#include <scenex/3d/static_mesh.hpp>
#include <scenex/3d/camera_3d.hpp>
#include <scenex/3d/transform_3d.hpp>
#include <scenex/systems/game_time.h>
#include <scenex/3d/render_system_3d.hpp>
#include <scenex/components/node_t.h>
#include <scenex/utility/scene_management.h>
#include <scenex/3d/light_3d.hpp>
#include <ek/math/rand.hpp>
#include <scenex/data/sg_factory.h>
#include <scenex/asset2/asset_manager.hpp>
#include <scenex/asset2/builtin_assets.hpp>

namespace ek {
void main() {
    setup_application_window({"Piko",
                              1024,
                              768});
    start_application();
}
}

using namespace scenex;
using namespace ek;

PikoApp::PikoApp()
        : basic_application()
#ifdef EK_EDITOR
, editor_{*this}
#endif
{
}

void PikoApp::initialize() {
    basic_application::initialize();
}

static ecs::entity main_camera{};
static ecs::entity main_scene_3d{};

struct test_rotation_comp {
};

void create_test_material(const std::string& name, argb32_t color, float roughness) {
    auto m = new material_3d;
    m->set_base_color(color);
    m->roughness = roughness;
    asset_t<material_3d>{name}.reset(m);
}

void create_lights() {
    auto e_dir_light = ecs::create<node_t, transform_3d, light_3d, mesh_renderer_component>();
    set_name(e_dir_light, "light_dir");
    ecs::get<light_3d>(e_dir_light).type = light_3d_type::directional;
    ecs::get<mesh_renderer_component>(e_dir_light).mesh = "sphere";
    ecs::get<mesh_renderer_component>(e_dir_light).material = "light_material";
    ecs::get<mesh_renderer_component>(e_dir_light).cast_shadows = false;
    ecs::get<mesh_renderer_component>(e_dir_light).receive_shadows = false;
    ecs::get<transform_3d>(e_dir_light).scale = {5.0f, 5.0f, 1.0f};
    ecs::get<transform_3d>(e_dir_light).position = {-50.0f, -85.0f, 40.0f};
    append(main_scene_3d, e_dir_light);

    auto e_light = ecs::create<node_t, transform_3d, light_3d, mesh_renderer_component>();
    set_name(e_light, "light");
    ecs::get<transform_3d>(e_light).position = {0.0f, 0.0f, 15.0f};
    ecs::get<light_3d>(e_light).diffuse = {1, 1, 1};
    ecs::get<mesh_renderer_component>(e_light).mesh = "sphere";
    ecs::get<mesh_renderer_component>(e_light).material = "light_material";
    ecs::get<mesh_renderer_component>(e_light).cast_shadows = false;
    ecs::get<mesh_renderer_component>(e_light).receive_shadows = false;
    append(main_scene_3d, e_light);
}

void create_coordinate_system_gizmo() {
    static auto cube_x = new static_mesh_t(create_cube(float3::zero, float3::one, 0xFF0000_rgb));
    static auto cube_y = new static_mesh_t(create_cube(float3::zero, float3::one, 0x00FF00_rgb));
    static auto cube_z = new static_mesh_t(create_cube(float3::zero, float3::one, 0x0000FF_rgb));

    auto e = ecs::create<node_t, transform_3d>();
    set_name(e, "cs");
    ecs::get<transform_3d>(e).scale = {5.0f, 5.0f, 5.0f};
    append(main_scene_3d, e);

    auto axis_size = 5.0f;
    auto ax = ecs::create<node_t, transform_3d, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(ax).mesh_ptr = cube_x;
    ecs::get<mesh_renderer_component>(ax).material = "light_material";
    ecs::get<transform_3d>(ax).position = float3{0.5f * axis_size, 0.0f, 0.0f};
    ecs::get<transform_3d>(ax).scale = float3{axis_size, 0.1f, 0.1f};
    append(e, ax);

    auto ay = ecs::create<node_t, transform_3d, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(ay).mesh_ptr = cube_y;
    ecs::get<mesh_renderer_component>(ay).material = "light_material";
    ecs::get<transform_3d>(ay).position = float3{0.0f, 0.5f * axis_size, 0.0f};
    ecs::get<transform_3d>(ay).scale = float3{0.1f, axis_size, 0.1f};
    append(e, ay);

    auto az = ecs::create<node_t, transform_3d, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(az).mesh_ptr = cube_z;
    ecs::get<mesh_renderer_component>(az).material = "light_material";
    ecs::get<transform_3d>(az).position = float3{0.0f, 0.0f, 0.5f * axis_size};
    ecs::get<transform_3d>(az).scale = float3{0.1f, 0.1f, axis_size};
    append(e, az);
}

void PikoApp::preload() {
    basic_application::preload();

    auto light_material = new material_3d;
    light_material->emission = float3::one;
    asset_t<material_3d>{"light_material"}.reset(light_material);

    static const char* test_materials[4] = {"test0",
                                            "test1",
                                            "test2",
                                            "test3"};
    static const char* test_models[4] = {"torus",
                                         "monkey",
                                         "sphere",
                                         "cube"};
    create_test_material("test0", 0xFFFF0000_argb, 0.05f);
    create_test_material("test1", 0xFF00FF00_argb, 0.1f);
    create_test_material("test2", 0xFF00FFFF_argb, 0.2f);
    create_test_material("test3", 0xFFFFFF00_argb, 0.3f);
    create_test_material("ground", 0xFF77FF77_argb, 0.01f);

#ifndef EK_EDITOR
    auto* asset_pack = asset_manager_->add_from_type("pack", "pack_meta");
    if (asset_pack) {
        asset_pack->load();
    }
#endif
    clear_color_enabled = true;

//    std::array<image_t*, 6> skybox{};
//    skybox[0] = load_image("assets/skybox/right.jpg"); //  +X
//    skybox[1] = load_image("assets/skybox/left.jpg"); //   -X
//    skybox[2] = load_image("assets/skybox/top.jpg"); //  +Y
//    skybox[3] = load_image("assets/skybox/bottom.jpg"); //   -Y
//    skybox[4] = load_image("assets/skybox/front.jpg"); //    +Z
//    skybox[5] = load_image("assets/skybox/back.jpg"); // -Z
//    auto* t = new texture_t(true);
//    t->upload_cubemap(skybox);
//    asset_t<texture_t>{"skybox"}.reset(t);

//    load_particles();
    piko::init(game);
//    setup_game(w, game);

//    asset_t<static_mesh_t>{"torus"}.reset(new static_mesh_t(load_obj(get_resource_content("assets/torus.obj"))));
//    asset_t<static_mesh_t>{"monkey"}.reset(new static_mesh_t(load_obj(get_resource_content("assets/monkey.obj"))));
//    asset_t<static_mesh_t>{"sphere"}.reset(new static_mesh_t(load_obj(get_resource_content("assets/sphere.obj"))));
    asset_t<static_mesh_t>{"cube"}.reset(new static_mesh_t(create_cube(float3::zero, float3::one)));

    main_scene_3d = ecs::create<node_t, transform_3d>();
    set_name(main_scene_3d, "scene 3d");

    main_camera = ecs::create<node_t, camera_3d, transform_3d, camera_arc_ball>();
    set_name(main_camera, "main camera");
    auto& camera_transform = ecs::get<transform_3d>(main_camera);
    camera_transform.position = {-100.0f, -100.0f, 100.0f};
    append(main_scene_3d, main_camera);
    ecs::get<camera_3d>(main_camera).cube_map = "skybox";
    ecs::get<camera_3d>(main_camera).clear_color_enabled = false;

    create_lights();

    auto e_ground = ecs::create<node_t, transform_3d, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(e_ground).mesh = "cube";
    ecs::get<mesh_renderer_component>(e_ground).material = "ground";

    set_name(e_ground, "ground");
    ecs::get<transform_3d>(e_ground).scale = {250.0f, 250.0f, 10.0f};
    ecs::get<transform_3d>(e_ground).position = {0.0f, 0.0f, -5.0f};
    append(main_scene_3d, e_ground);

    for (int i = 0; i < 20; ++i) {
        auto es = ecs::create<node_t, transform_3d, mesh_renderer_component, test_rotation_comp>();
        ecs::get<mesh_renderer_component>(es).mesh = "sphere";
        auto mat_id = "rr" + std::to_string(i);
        auto r = static_cast<float>(i) / 20.0f;
        create_test_material(mat_id,
                             0xFF0000_rgb,
                             math::lerp(0.001f, 0.33f, r));
        ecs::get<mesh_renderer_component>(es).material = mat_id;
        ecs::get<transform_3d>(es).position = {
                math::lerp(-100.0f, 100.0f, r),
                0.0f,
                5.0f
        };
        ecs::get<transform_3d>(es).scale = 4.0f * float3::one;
        append(main_scene_3d, es);
    }
    for (int i = 0; i < 10; ++i) {
        auto e_cube = ecs::create<node_t, transform_3d, mesh_renderer_component, test_rotation_comp>();
        ecs::get<mesh_renderer_component>(e_cube).mesh = test_models[rand_fx.random_int(0, 3)];
        ecs::get<mesh_renderer_component>(e_cube).material = test_materials[rand_fx.random_int(0, 3)];
        set_name(e_cube, "cube");
        ecs::get<transform_3d>(e_cube).position = {
                rand_fx.random(-50.0f, 50.0f),
                rand_fx.random(-50.0f, 50.0f),
                rand_fx.random(5.0f, 15.0f)
        };

        const float tor_scale = rand_fx.random(1.0f, 5.0f);
        ecs::get<transform_3d>(e_cube).scale = tor_scale * float3::one;
        ecs::get<transform_3d>(e_cube).rotation = {
                rand_fx.random(0.0f, 180.0f),
                rand_fx.random(0.0f, 180.0f),
                rand_fx.random(0.0f, 180.0f)
        };
        append(main_scene_3d, e_cube);
    }

    append(game, sg_create("tests", "test"));
}

void PikoApp::update_frame(float dt) {
    basic_application::update_frame(dt);

    scene_pre_update(root, dt);

    update_camera_arc_ball(dt);

    piko::update(game);
    //update_game_title_start(w);
    scene_post_update(root, dt);
}

void PikoApp::render_frame() {
//    basic_application::render_frame();

    const float dt = get_delta_time(root);

    for (auto e : ecs::view<test_rotation_comp>()) {
        auto& tr = ecs::get<transform_3d>(e);
        tr.rotation.x += dt;
        if (tr.rotation.x > math::pi2) {
            tr.rotation.x -= math::pi2;
        }
        tr.rotation.y += dt * 2;
        if (tr.rotation.y > math::pi2) {
            tr.rotation.y -= math::pi2;
        }
    }

    auto light = find(main_scene_3d, "light");
    if (light) {
        static float lt = 0.0f;
        lt += dt;
        ecs::get<transform_3d>(light).position = {
                50.0f * cos(lt),
                50.0f * sin(lt),
                15.0f
        };
    }

    render_3d_scene(main_scene_3d, main_camera);

    draw_node(root);
}

PikoApp::~PikoApp() = default;

void ek_main() {
    run_app<PikoApp>();
}
