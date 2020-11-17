#include "UglyFilter2D.hpp"

#include <ek/math/common.hpp>
#include <ek/draw2d/drawer.hpp>

namespace ek {

using ecs::world;
using ecs::entity;

// TODO: draw pass without world transform could be done??
bool UglyFilter2D::process(entity e) {
    auto* uglyFilter = e.tryGet<UglyFilter2D>();
    if (uglyFilter && uglyFilter->enabled && !uglyFilter->processing) {
        for (auto& filter : uglyFilter->filters) {
            if (filter.type == sg_filter_type::drop_shadow) {
                uglyFilter->processing = true;
                draw2d::state.save_transform()
                        .translate(filter.offset);
                draw2d::state.color.scale = argb32_t{0x0u, filter.color.af()};
                draw2d::state.color.offset = argb32_t{filter.color.argb, 0.0f};

                // TODO:
                //draw_node(e);

                draw2d::state.restore_transform();

                uglyFilter->processing = false;
            } else if (filter.type == sg_filter_type::glow) {
                uglyFilter->processing = true;
                draw2d::state.save_color();
                draw2d::state.color.scale = argb32_t{0x0u, filter.color.af()};
                draw2d::state.color.offset = argb32_t{filter.color.argb, 0.0f};

                const int segments = std::min(12,
                                              8 * std::max(int(std::ceil((filter.blur.x + filter.blur.y)) / 2.0f), 1));
                const auto da = float(math::pi2 / segments);
                auto a = 0.0f;
                for (int i = 0; i < segments; ++i) {
                    draw2d::state.save_matrix()
                            .translate(filter.blur * float2{std::cos(a), std::sin(a)});

                    // TODO:
                    //draw_node(e);

                    draw2d::state.restore_matrix();
                    a += da;
                }
                draw2d::state.restore_color();
                uglyFilter->processing = false;
            }
        }
    }
    return false;
}

}
