#include "node_filters.hpp"

#include <ek/scenex/scene_system.hpp>
#include <ek/math/common.hpp>

namespace ek {

using ecs::world;
using ecs::entity;

bool process_node_filters(entity e) {
    if (ecs::has<node_filters_t>(e)) {
        auto& filters = ecs::get<node_filters_t>(e);
        if (filters.enabled && !filters.processing) {
            for (auto& data : filters.filters) {
                if (data.type == sg_filter_type::drop_shadow) {
                    filters.processing = true;
                    auto& drawer = resolve<drawer_t>();
                    drawer.state.save_transform()
                            .translate(data.offset);
                    drawer.state.color_multiplier = argb32_t{0x0u, data.color.af()};
                    drawer.state.color_offset = argb32_t{data.color.argb, 0.0f};

                    draw_node(e);
                    drawer.state.restore_transform();

                    filters.processing = false;
                } else if (data.type == sg_filter_type::glow) {
                    filters.processing = true;
                    auto& drawer = resolve<drawer_t>();
                    drawer.state.save_color();
                    drawer.state.color_multiplier = argb32_t{0x0u, data.color.af()};
                    drawer.state.color_offset = argb32_t{data.color.argb, 0.0f};

                    const int segments = std::min(12,
                                                  8 * std::max(int(std::ceil((data.blur.x + data.blur.y)) / 2.0f), 1));
                    const auto da = float(math::pi2 / segments);
                    auto a = 0.0f;
                    for (int i = 0; i < segments; ++i) {
                        drawer.state.save_matrix()
                                .translate(data.blur * float2{std::cos(a), std::sin(a)});
                        draw_node(e);
                        drawer.state.restore_matrix();
                        a += da;
                    }
                    drawer.state.restore_color();
                    filters.processing = false;
                }
            }

        }
    }
    return false;
}

}
