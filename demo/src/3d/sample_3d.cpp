#include <ek/scenex/3d/Material3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/timers.hpp>
#include <ek/scenex/3d/RenderSystem3D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/3d/StaticMesh.hpp>
#include <ek/util/Res.hpp>

#include <demo_main.hpp>
#include <ek/math/rand.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
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
    auto e_dir_light = ecs::create<Node, Transform3D, Light3D, MeshRenderer>();
    setName(e_dir_light, "light_dir");
    e_dir_light.get<Light3D>().type = light_3d_type::directional;
    e_dir_light.get<MeshRenderer>().mesh = "sphere";
    e_dir_light.get<MeshRenderer>().material = "light_material";
    e_dir_light.get<MeshRenderer>().castShadows = false;
    e_dir_light.get<MeshRenderer>().receiveShadows = false;
    e_dir_light.get<Transform3D>().scale = {5.0f, 5.0f, 1.0f};
    e_dir_light.get<Transform3D>().position = {-50.0f, -85.0f, 40.0f};
    append(main_scene_3d, e_dir_light);

    auto e_light = ecs::create<Node, Transform3D, Light3D, MeshRenderer>();
    setName(e_light, "light");
    e_light.get<Transform3D>().position = {0.0f, 0.0f, 15.0f};
    e_light.get<Light3D>().diffuse = {1, 1, 1};
    e_light.get<MeshRenderer>().mesh = "sphere";
    e_light.get<MeshRenderer>().material = "light_material";
    e_light.get<MeshRenderer>().castShadows = false;
    e_light.get<MeshRenderer>().receiveShadows = false;
    append(main_scene_3d, e_light);
}

void create_coordinate_system_gizmo() {
    static auto cube_x = new StaticMesh(MeshData::createCube(float3::zero, float3::one, 0xFF0000_rgb));
    static auto cube_y = new StaticMesh(MeshData::createCube(float3::zero, float3::one, 0x00FF00_rgb));
    static auto cube_z = new StaticMesh(MeshData::createCube(float3::zero, float3::one, 0x0000FF_rgb));

    auto e = ecs::create<Node, Transform3D>();
    setName(e, "cs");
    e.get<Transform3D>().scale = {5.0f, 5.0f, 5.0f};
    append(main_scene_3d, e);

    auto axis_size = 5.0f;
    auto ax = ecs::create<Node, Transform3D, MeshRenderer>();
    ax.get<MeshRenderer>().meshPtr = cube_x;
    ax.get<MeshRenderer>().material = "light_material";
    ax.get<Transform3D>().position = float3{0.5f * axis_size, 0.0f, 0.0f};
    ax.get<Transform3D>().scale = float3{axis_size, 0.1f, 0.1f};
    append(e, ax);

    auto ay = ecs::create<Node, Transform3D, MeshRenderer>();
    ay.get<MeshRenderer>().meshPtr = cube_y;
    ay.get<MeshRenderer>().material = "light_material";
    ay.get<Transform3D>().position = float3{0.0f, 0.5f * axis_size, 0.0f};
    ay.get<Transform3D>().scale = float3{0.1f, axis_size, 0.1f};
    append(e, ay);

    auto az = ecs::create<Node, Transform3D, MeshRenderer>();
    az.get<MeshRenderer>().meshPtr = cube_z;
    az.get<MeshRenderer>().material = "light_material";
    az.get<Transform3D>().position = float3{0.0f, 0.0f, 0.5f * axis_size};
    az.get<Transform3D>().scale = float3{0.1f, 0.1f, axis_size};
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
        auto& tr = e.get<Transform3D>();
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
            light.get<Transform3D>().position = {
                    50.0f * cos(lt),
                    50.0f * sin(lt),
                    15.0f
            };
        }
    }
}

Sample3D::Sample3D() {
    auto* rs3d = try_resolve<RenderSystem3D>();
    if (!rs3d) {
        service_locator_instance<RenderSystem3D>::init();
        rs3d = try_resolve<RenderSystem3D>();
    }

    title = "SCENE 3D";
    Camera2D::Main.get<Camera2D>().clearColorEnabled = false;

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
    Res<StaticMesh>{"cube"}.reset(new StaticMesh(MeshData::createCube(float3::zero, float3::one)));

    main_scene_3d = ecs::create<Node, Transform3D>();
    setName(main_scene_3d, "scene 3d");

    main_camera = ecs::create<Node, Camera3D, Transform3D, camera_arc_ball>();
    setName(main_camera, "main camera");

    rs3d->scene = main_scene_3d;
    rs3d->camera = main_camera;

    auto& camera_transform = main_camera.get<Transform3D>();
    camera_transform.position = {-100.0f, -100.0f, 100.0f};
    append(main_scene_3d, main_camera);
    main_camera.get<Camera3D>().cubeMap.setID("skybox");
    main_camera.get<Camera3D>().clearColorEnabled = false;

    create_lights();

    auto e_ground = ecs::create<Node, Transform3D, MeshRenderer>();
    e_ground.get<MeshRenderer>().mesh = "cube";
    e_ground.get<MeshRenderer>().material = "ground";

    setName(e_ground, "ground");
    e_ground.get<Transform3D>().scale = {250.0f, 250.0f, 10.0f};
    e_ground.get<Transform3D>().position = {0.0f, 0.0f, -5.0f};
    append(main_scene_3d, e_ground);

    for (int i = 0; i < 20; ++i) {
        auto es = ecs::create<Node, Transform3D, MeshRenderer, test_rotation_comp>();
        es.get<MeshRenderer>().mesh = "sphere";
        auto mat_id = "rr" + std::to_string(i);
        auto r = static_cast<float>(i) / 20.0f;
        create_test_material(mat_id,
                             0xFF0000_rgb,
                             math::lerp(0.001f, 0.33f, r));
        es.get<MeshRenderer>().material = mat_id;
        es.get<Transform3D>().position = {
                math::lerp(-100.0f, 100.0f, r),
                0.0f,
                5.0f
        };
        es.get<Transform3D>().scale = 4.0f * float3::one;
        append(main_scene_3d, es);
    }
    for (int i = 0; i < 10; ++i) {
        auto e_cube = ecs::create<Node, Transform3D, MeshRenderer, test_rotation_comp>();
        e_cube.get<MeshRenderer>().mesh = test_models[rand_fx.random_int(0, 3)];
        e_cube.get<MeshRenderer>().material = test_materials[rand_fx.random_int(0, 3)];
        setName(e_cube, "cube");
        e_cube.get<Transform3D>().position = {
                rand_fx.random(-50.0f, 50.0f),
                rand_fx.random(-50.0f, 50.0f),
                rand_fx.random(5.0f, 15.0f)
        };

        const float tor_scale = rand_fx.random(1.0f, 5.0f);
        e_cube.get<Transform3D>().scale = tor_scale * float3::one;
        e_cube.get<Transform3D>().rotation = {
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
    Camera2D::Main.get<Camera2D>().clearColorEnabled = true;
}

}