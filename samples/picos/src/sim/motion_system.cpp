#include "motion_system.h"

#include <sim/components/attractor.h>
#include <sim/components/position.h>
#include <sim/components/motion.h>
#include <ek/scenex/components/transform_2d.hpp>

namespace ek::piko {

void update_motion_system(float dt) {

    auto attractors = ecs::view<attractor_t, transform_2d>();

    ecs::view<position_t, motion_t, transform_2d>().each(
            [dt, &attractors](position_t& pos, motion_t& mot, transform_2d& tra) {

                auto p = pos.position;
                auto v = mot.velocity;

                for (auto attractor_entity : attractors) {
                    auto pointer = ecs::get<transform_2d>(attractor_entity).matrix.position();
                    auto& attractor = ecs::get<attractor_t>(attractor_entity);
                    float factor = 1.0f - math::clamp(length(pointer - p) / attractor.radius);
                    v += dt * attractor.force * factor * factor * normalize(pointer - p);
                }

                p += dt * mot.velocity;

                const rect_f bounds{0.0f, 0.0f, 720.0f, 960.0f};
                if (p.x < bounds.x) {
                    v.x = -v.x;
                    p.x = bounds.x;
                } else if (p.x > bounds.right()) {
                    v.x = -v.x;
                    p.x = bounds.right();
                }
                if (p.y < bounds.y) {
                    v.y = -v.y;
                    p.y = bounds.y;
                } else if (p.y > bounds.bottom()) {
                    v.y = -v.y;
                    p.y = bounds.bottom();
                }

                v *= expf(-6.0f * dt);
                pos.position = p;
                mot.velocity = v;
                tra.matrix.position(p);
            }
    );
}

}