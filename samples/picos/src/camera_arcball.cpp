#include "camera_arcball.hpp"

#include <ecxx/ecxx.hpp>
#include <scenex/3d/camera_3d.hpp>
#include <scenex/3d/transform_3d.hpp>
#include <scenex/interactive_manager.h>
#include <ek/util/locator.hpp>
#include <scenex/ek/input_controller.h>
#include <scenex/systems/game_time.h>
#include <ek/util/timer.hpp>

using namespace scenex;
using namespace ek::app;

namespace ek {

void update_camera_arc_ball(float dt) {

    auto& im = resolve<interactive_manager>();
    static float2 prev_pointer{};
    static bool prev_down = false;

    float2 delta{};
    if (im.pointer_down && !prev_down) {
        prev_down = true;
        prev_pointer = im.pointer_global_space;
    }
    if (!im.pointer_down && prev_down) {
        prev_down = false;
    }
    if (prev_down) {
        float2 cur = im.pointer_global_space;
        delta = cur - prev_pointer;
        prev_pointer = cur;
    }
    for (auto e: ecs::view<camera_3d, camera_arc_ball, transform_3d>()) {
        auto& arc_ball = ecs::get<camera_arc_ball>(e);
        auto& camera_data = ecs::get<camera_3d>(e);
        auto& camera_transform = ecs::get<transform_3d>(e);

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
        if (input.is_key(key_code::W)) {
            dd -= 10.0f * dt;
        }
        if (input.is_key(key_code::S)) {
            dd += 10.0f * dt;
        }
        arc_ball.distance = std::max(arc_ball.distance + dd, 0.0f);


        dir = normalize(arc_ball.center - camera_transform.position);
        camera_transform.position = arc_ball.center - arc_ball.distance * dir;
        camera_transform.rotation = euler_angles(quat_look_at_rh(dir, camera_data.up));
    }
}

}