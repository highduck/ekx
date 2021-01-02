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

    draw2d::current().save_transform();

    for (auto& filter : filters) {
        if (filter.type == SGFilterType::DropShadow) {
            processing = true;
            draw2d::current().matrix = parentMatrix;
            draw2d::current().color = parentColor;

            draw2d::current().concat(argb32_t{0x0u, filter.color.af()}, argb32_t{filter.color.argb, 0.0f});
            draw2d::current().translate(filter.offset);

            if (localTransform) {
                draw2d::current().concat(localTransform->matrix);
            }

            RenderSystem2D::drawStack(e);

            processing = false;
        } else if (filter.type == SGFilterType::Glow) {
            processing = true;
            draw2d::current().color = parentColor;
            draw2d::current().concat(argb32_t{0x0u, filter.color.af()}, argb32_t{filter.color.argb, 0.0f});

            const int segments = std::min(12,
                                          8 * std::max(int(std::ceil((filter.blur.x + filter.blur.y)) / 2.0f), 1));
            const auto da = float(math::pi2 / segments);
            auto a = 0.0f;
            for (int i = 0; i < segments; ++i) {
                draw2d::current().matrix = parentMatrix;

                draw2d::current().translate(filter.blur * float2{std::cos(a), std::sin(a)});
                if (localTransform) {
                    draw2d::current().concat(localTransform->matrix);
                }

                RenderSystem2D::drawStack(e);
                a += da;
            }
            processing = false;
        }
    }
    draw2d::current().restore_transform();
    return false;
}

}