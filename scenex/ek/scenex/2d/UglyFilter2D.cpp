#include "UglyFilter2D.hpp"
#include "RenderSystem2D.hpp"
#include "Transform2D.hpp"

#include <ek/math/common.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

using ecs::entity;

bool UglyFilter2D::pass(entity e) {
    auto* localTransform = e.tryGet<Transform2D>();
    auto* parentTransform = findComponentInParent<Transform2D>(e.get<Node>().parent);
    assert(parentTransform);
    auto parentMatrix = parentTransform->worldMatrix;
    auto parentColor = parentTransform->worldColor;

    draw2d::state.save_transform();

    for (auto& filter : filters) {
        if (filter.type == sg_filter_type::drop_shadow) {
            processing = true;
            draw2d::state.matrix = parentMatrix;
            draw2d::state.color = parentColor;

            draw2d::state.concat(argb32_t{0x0u, filter.color.af()}, argb32_t{filter.color.argb, 0.0f});
            draw2d::state.translate(filter.offset);

            if (localTransform) {
                draw2d::state.concat(localTransform->matrix);
            }

            RenderSystem2D::drawStack(e);

            processing = false;
        } else if (filter.type == sg_filter_type::glow) {
            processing = true;
            draw2d::state.color = parentColor;
            draw2d::state.concat(argb32_t{0x0u, filter.color.af()}, argb32_t{filter.color.argb, 0.0f});

            const int segments = std::min(12,
                                          8 * std::max(int(std::ceil((filter.blur.x + filter.blur.y)) / 2.0f), 1));
            const auto da = float(math::pi2 / segments);
            auto a = 0.0f;
            for (int i = 0; i < segments; ++i) {
                draw2d::state.matrix = parentMatrix;

                draw2d::state.translate(filter.blur * float2{std::cos(a), std::sin(a)});
                if (localTransform) {
                    draw2d::state.concat(localTransform->matrix);
                }

                RenderSystem2D::drawStack(e);
                a += da;
            }
            processing = false;
        }
    }
    draw2d::state.restore_transform();
    return false;
}

}