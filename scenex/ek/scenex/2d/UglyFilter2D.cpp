#include "UglyFilter2D.hpp"
#include "RenderSystem2D.hpp"
#include "Transform2D.hpp"

#include <ek/math/common.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

bool UglyFilter2D::pass(const ecs::world& w, ecs::Entity e) {
    auto* localTransform = w.tryGet<Transform2D>(e);
    auto* parentTransform = findComponentInParent<WorldTransform2D>(w.get<Node>(e).parent);
    assert(parentTransform);
    auto parentMatrix = parentTransform->matrix;
    auto parentColor = parentTransform->color;

    draw2d::state.save_transform();

    for (auto& filter : filters) {
        if (filter.type == SGFilterType::DropShadow) {
            processing = true;
            draw2d::state.matrix = parentMatrix;
            draw2d::state.color = parentColor;

            draw2d::state.concat(argb32_t{0x0u, filter.color.af()}, argb32_t{filter.color.argb, 0.0f});
            draw2d::state.translate(filter.offset);

            if (localTransform) {
                draw2d::state.concat(localTransform->matrix);
            }

            RenderSystem2D::drawStack(w, e);

            processing = false;
        } else if (filter.type == SGFilterType::Glow) {
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

                RenderSystem2D::drawStack(w, e);
                a += da;
            }
            processing = false;
        }
    }
    draw2d::state.restore_transform();
    return false;
}

}