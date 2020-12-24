#include "camera_arcball.hpp"

#include <ecxx/ecxx.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/util/locator.hpp>
#include <ek/scenex/app/input_controller.hpp>
#include <ek/math/quaternion.hpp>

namespace ek {

void update_camera_arc_ball(float dt) {

    auto& im = resolve<InteractionSystem>();
    static float2 prev_pointer{};
    static bool prev_down = false;

    float2 delta{};
    if (im.pointerDown_ && !prev_down) {
        prev_down = true;
        prev_pointer = im.pointerScreenPosition_;
    }
    if (!im.pointerDown_ && prev_down) {
        prev_down = false;
    }
    if (prev_down) {
        float2 cur = im.pointerScreenPosition_;
        delta = cur - prev_pointer;
        prev_pointer = cur;
    }
    for (auto e: ecs::view<Camera3D, camera_arc_ball, Transform3D>()) {
        auto& arc_ball = ecs::get<camera_arc_ball>(e);
        auto& camera_data = ecs::get<Camera3D>(e);
        auto& camera_transform = ecs::get<Transform3D>(e);

        auto dir = normalize(arc_ball.center - camera_transform.position);
        auto r = cross(dir, camera_data.up);
        auto t = cross(r, dir);
        if (length(r) < 0.001f) {
            t = cross(dir, float3{0, 1, 0});
            r = -cross(t, dir);
        }

        camera_transform.position += -r * delta.x + t * delta.y;

        auto dd = 0.0f;
        auto& input = resolve<input_controller>();
        if (input.is_key(app::key_code::W)) {
            dd -= 10.0f * dt;
        }
        if (input.is_key(app::key_code::S)) {
            dd += 10.0f * dt;
        }
        arc_ball.distance = std::max(arc_ball.distance + dd, 0.0f);

        dir = normalize(arc_ball.center - camera_transform.position);
        camera_transform.position = arc_ball.center - arc_ball.distance * dir;
        camera_transform.rotation = euler_angles(quat_look_at_rh(dir, camera_data.up));
    }
}

}