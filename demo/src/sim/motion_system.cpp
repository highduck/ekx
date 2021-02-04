#include "motion_system.h"

#include <sim/components/attractor.h>
#include <sim/components/motion.h>
#include <ek/scenex/2d/Transform2D.hpp>

namespace ek::piko {

inline const float WIDTH = 360;
inline const float HEIGHT = 480;

struct AttractorsState {
    attractor_t props;
    float2 position;
};

void update_motion_system(float dt) {
    static std::vector<AttractorsState> attractors{};
    const auto& w = ecs::the_world;
    attractors.clear();
    for (auto e_ : ecs::view<attractor_t>()) {
        const auto e = e_.index;
        attractors.emplace_back(AttractorsState{
            w.get<attractor_t>(e),
            w.get<Transform2D>(e).getPosition()
        });
    }
    const auto attractorsCount = static_cast<uint32_t>(attractors.size());

    const auto dumpFactor = expf(-6.0f * dt);
    const rect_f bounds{0.0f, 0.0f, WIDTH, HEIGHT};
    for (auto e_ : ecs::view<motion_t>()) {
        auto e = e_.index;
        auto& mot = w.get<motion_t>(e);
        auto& tra = w.get<Transform2D>(e);

        auto p = tra.getPosition();
        auto v = mot.velocity;

        for (uint32_t i = 0; i < attractorsCount; ++i) {
            auto& attractor = attractors[i];
            auto diff = attractor.position - p;
            auto len = length(diff);
            float factor = 1.0f - math::clamp(len / attractor.props.radius);
            v += dt * attractor.props.force * factor * factor * diff * (1.0f / len);
        }

        p += dt * mot.velocity;

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

        mot.velocity = v * dumpFactor;
        tra.setPosition(p);
    }
}

}