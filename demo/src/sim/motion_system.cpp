#include "motion_system.h"

#include <sim/components/attractor.h>
#include <sim/components/motion.h>
#include <ek/scenex/2d/Transform2D.hpp>

namespace ek::piko {

inline const float WIDTH = 360;
inline const float HEIGHT = 480;

void update_motion_system(float dt) {

    auto attractors = ecs::view<attractor_t>();

    ecs::view<motion_t, Transform2D>().each(
            [dt, &attractors](motion_t& mot, Transform2D& tra) {

                auto p = tra.position;
                auto v = mot.velocity;

                for (auto attractor_entity : attractors) {
                    auto pointer = attractor_entity.get<Transform2D>().position;
                    auto& attractor = attractor_entity.get<attractor_t>();
                    float factor = 1.0f - math::clamp(length(pointer - p) / attractor.radius);
                    v += dt * attractor.force * factor * factor * normalize(pointer - p);
                }

                p += dt * mot.velocity;

                const rect_f bounds{0.0f, 0.0f, WIDTH, HEIGHT};
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
                mot.velocity = v;
                tra.position = p;
            }
    );
}

}