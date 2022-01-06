#include "UglyFilter2D.hpp"
#include "RenderSystem2D.hpp"
#include "Transform2D.hpp"

#include <ek/math/Math.hpp>
#include <ek/canvas.h>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/data/SGFile.hpp>

namespace ek {

bool UglyFilter2D::pass(const ecs::World& w, ecs::EntityIndex e) {
    auto* localTransform = w.tryGet<Transform2D>(e);
    auto* parentTransform = findComponentInParent<WorldTransform2D>(w.get<Node>(e).parent);
    EK_ASSERT(parentTransform);
    auto parentMatrix = parentTransform->matrix;
    auto parentColor = parentTransform->color;

    canvas_save_transform();

    for (auto& filter : filters) {
        if (filter.type == SGFilterType::DropShadow) {
            processing = true;
            canvas.matrix[0] = parentMatrix;
            canvas.color[0] = parentColor;

            canvas_concat_color({
                argb32_t{0x0u, filter.color.af()},
                argb32_t{filter.color.argb, 0.0f}
            });
            canvas_translate(filter.offset);

            if (localTransform) {
                canvas_concat_matrix(localTransform->matrix);
            }

            RenderSystem2D::drawStack(w, e);

            processing = false;
        } else if (filter.type == SGFilterType::Glow) {
            processing = true;
            canvas.color[0] = parentColor;
            canvas_concat_color({
                argb32_t{0x0u, filter.color.af()},
                argb32_t{filter.color.argb, 0.0f}
            });

            const int segments = std::min(12,
                                          8 * std::max(int(std::ceil((filter.blur.x + filter.blur.y)) / 2.0f), 1));
            const auto da = float(Math::pi2 / segments);
            auto a = 0.0f;
            for (int i = 0; i < segments; ++i) {
                canvas.matrix[0] = parentMatrix;

                canvas_translate(filter.blur * Vec2f{std::cos(a), std::sin(a)});
                if (localTransform) {
                    canvas_concat_matrix(localTransform->matrix);
                }

                RenderSystem2D::drawStack(w, e);
                a += da;
            }
            processing = false;
        }
    }
    canvas_restore_transform();
    return false;
}

}