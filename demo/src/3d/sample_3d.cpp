#include <ek/scenex/3d/Material3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/game_time.hpp>
#include <ek/scenex/3d/render_system_3d.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/3d/static_mesh.hpp>
#include <ek/util/assets.hpp>

#include <demo_main.hpp>
#include <ek/math/rand.hpp>
#include "sample_3d.hpp"
#include "camera_arcball.hpp"

namespace ek {

static ecs::entity main_camera{};
static ecs::entity main_scene_3d{};

static const char* test_materials[4] = {"test0",
                                        "test1",
                                        "test2",
                                        "test3"};
static const char* test_models[4] = {"torus",
                                     "monkey",
                                     "sphere",
                                     "cube"};

struct test_rotation_comp {
};

void create_test_material(const std::string& name, argb32_t color, float roughness) {
    auto m = new Material3D;
    m->set_base_color(color);
    m->roughness = roughness;
    Res<Material3D>{name}.reset(m);
}

void create_lights() {
    auto e_dir_light = ecs::create<Node, Transform3D, Light3D, mesh_renderer_component>();
    setName(e_dir_light, "light_dir");
    ecs::get<Light3D>(e_dir_light).type = light_3d_type::directional;
    ecs::get<mesh_renderer_component>(e_dir_light).mesh = "sphere";
    ecs::get<mesh_renderer_component>(e_dir_light).material = "light_material";
    ecs::get<mesh_renderer_component>(e_dir_light).cast_shadows = false;
    ecs::get<mesh_renderer_component>(e_dir_light).receive_shadows = false;
    ecs::get<Transform3D>(e_dir_light).scale = {5.0f, 5.0f, 1.0f};
    ecs::get<Transform3D>(e_dir_light).position = {-50.0f, -85.0f, 40.0f};
    append(main_scene_3d, e_dir_light);

    auto e_light = ecs::create<Node, Transform3D, Light3D, mesh_renderer_component>();
    setName(e_light, "light");
    ecs::get<Transform3D>(e_light).position = {0.0f, 0.0f, 15.0f};
    ecs::get<Light3D>(e_light).diffuse = {1, 1, 1};
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

    auto e = ecs::create<Node, Transform3D>();
    setName(e, "cs");
    ecs::get<Transform3D>(e).scale = {5.0f, 5.0f, 5.0f};
    append(main_scene_3d, e);

    auto axis_size = 5.0f;
    auto ax = ecs::create<Node, Transform3D, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(ax).mesh_ptr = cube_x;
    ecs::get<mesh_renderer_component>(ax).material = "light_material";
    ecs::get<Transform3D>(ax).position = float3{0.5f * axis_size, 0.0f, 0.0f};
    ecs::get<Transform3D>(ax).scale = float3{axis_size, 0.1f, 0.1f};
    append(e, ax);

    auto ay = ecs::create<Node, Transform3D, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(ay).mesh_ptr = cube_y;
    ecs::get<mesh_renderer_component>(ay).material = "light_material";
    ecs::get<Transform3D>(ay).position = float3{0.0f, 0.5f * axis_size, 0.0f};
    ecs::get<Transform3D>(ay).scale = float3{0.1f, axis_size, 0.1f};
    append(e, ay);

    auto az = ecs::create<Node, Transform3D, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(az).mesh_ptr = cube_z;
    ecs::get<mesh_renderer_component>(az).material = "light_material";
    ecs::get<Transform3D>(az).position = float3{0.0f, 0.0f, 0.5f * axis_size};
    ecs::get<Transform3D>(az).scale = float3{0.1f, 0.1f, axis_size};
    append(e, az);
}

void Sample3D::update(float dt) {
    SampleBase::update(dt);
    update_camera_arc_ball(dt);
}

void Sample3D::draw() {
    SampleBase::draw();

    auto& app = resolve<basic_application>();
    const float dt = TimeLayer::Root->dt;

    for (auto e : ecs::view<test_rotation_comp>()) {
        auto& tr = ecs::get<Transform3D>(e);
        tr.rotation.x += dt;
        if (tr.rotation.x > math::pi2) {
            tr.rotation.x -= math::pi2;
        }
        tr.rotation.y += dt * 2;
        if (tr.rotation.y > math::pi2) {
            tr.rotation.y -= math::pi2;
        }
    }

    if (main_scene_3d) {
        auto light = find(main_scene_3d, "light");
        if (light) {
            static float lt = 0.0f;
            lt += dt;
            ecs::get<Transform3D>(light).position = {
                    50.0f * cos(lt),
                    50.0f * sin(lt),
                    15.0f
            };
        }

        render_3d_scene(main_scene_3d, main_camera);
    }
}

Sample3D::Sample3D() {
    title = "SCENE 3D";

    auto light_material = new Material3D;
    light_material->emission = float3::one;
    Res<Material3D>{"light_material"}.reset(light_material);

    create_test_material("test0", 0xFFFF0000_argb, 0.05f);
    create_test_material("test1", 0xFF00FF00_argb, 0.1f);
    create_test_material("test2", 0xFF00FFFF_argb, 0.2f);
    create_test_material("test3", 0xFFFFFF00_argb, 0.3f);
    create_test_material("ground", 0xFF77FF77_argb, 0.01f);

    //    asset_t<static_mesh_t>{"torus"}.reset(new static_mesh_t(load_obj(get_resource_content("assets/torus.obj"))));
//    asset_t<static_mesh_t>{"monkey"}.reset(new static_mesh_t(load_obj(get_resource_content("assets/monkey.obj"))));
//    asset_t<static_mesh_t>{"sphere"}.reset(new static_mesh_t(load_obj(get_resource_content("assets/sphere.obj"))));
    Res<static_mesh_t>{"cube"}.reset(new static_mesh_t(create_cube(float3::zero, float3::one)));

    main_scene_3d = ecs::create<Node, Transform3D>();
    setName(main_scene_3d, "scene 3d");

    main_camera = ecs::create<Node, Camera3D, Transform3D, camera_arc_ball>();
    setName(main_camera, "main camera");
    auto& camera_transform = ecs::get<Transform3D>(main_camera);
    camera_transform.position = {-100.0f, -100.0f, 100.0f};
    append(main_scene_3d, main_camera);
    ecs::get<Camera3D>(main_camera).cubeMap.setID("skybox");
    ecs::get<Camera3D>(main_camera).clearColorEnabled = false;

    create_lights();

    auto e_ground = ecs::create<Node, Transform3D, mesh_renderer_component>();
    ecs::get<mesh_renderer_component>(e_ground).mesh = "cube";
    ecs::get<mesh_renderer_component>(e_ground).material = "ground";

    setName(e_ground, "ground");
    ecs::get<Transform3D>(e_ground).scale = {250.0f, 250.0f, 10.0f};
    ecs::get<Transform3D>(e_ground).position = {0.0f, 0.0f, -5.0f};
    append(main_scene_3d, e_ground);

    for (int i = 0; i < 20; ++i) {
        auto es = ecs::create<Node, Transform3D, mesh_renderer_component, test_rotation_comp>();
        ecs::get<mesh_renderer_component>(es).mesh = "sphere";
        auto mat_id = "rr" + std::to_string(i);
        auto r = static_cast<float>(i) / 20.0f;
        create_test_material(mat_id,
                             0xFF0000_rgb,
                             math::lerp(0.001f, 0.33f, r));
        ecs::get<mesh_renderer_component>(es).material = mat_id;
        ecs::get<Transform3D>(es).position = {
                math::lerp(-100.0f, 100.0f, r),
                0.0f,
                5.0f
        };
        ecs::get<Transform3D>(es).scale = 4.0f * float3::one;
        append(main_scene_3d, es);
    }
    for (int i = 0; i < 10; ++i) {
        auto e_cube = ecs::create<Node, Transform3D, mesh_renderer_component, test_rotation_comp>();
        ecs::get<mesh_renderer_component>(e_cube).mesh = test_models[rand_fx.random_int(0, 3)];
        ecs::get<mesh_renderer_component>(e_cube).material = test_materials[rand_fx.random_int(0, 3)];
        setName(e_cube, "cube");
        ecs::get<Transform3D>(e_cube).position = {
                rand_fx.random(-50.0f, 50.0f),
                rand_fx.random(-50.0f, 50.0f),
                rand_fx.random(5.0f, 15.0f)
        };

        const float tor_scale = rand_fx.random(1.0f, 5.0f);
        ecs::get<Transform3D>(e_cube).scale = tor_scale * float3::one;
        ecs::get<Transform3D>(e_cube).rotation = {
                rand_fx.random(0.0f, 180.0f),
                rand_fx.random(0.0f, 180.0f),
                rand_fx.random(0.0f, 180.0f)
        };
        append(main_scene_3d, e_cube);
    }
}

Sample3D::~Sample3D() {
    destroyNode(main_scene_3d);
    main_scene_3d = nullptr;
}

}