#include "LayoutRect.hpp"
#include "Display2D.hpp"
#include <ek/scenex/base/Node.hpp>
#include <ek/app/app.hpp>

namespace ek {

rect_f find_parent_layout_rect(ecs::EntityApi e, bool safe) {
    auto it = e.get<Node>().parent;
    while (it) {
        const auto* layout = it.tryGet<LayoutRect>();
        if (layout && !layout->rect.empty()) {
            return safe ? layout->safeRect : layout->rect;
        }
        it = it.get<Node>().parent;
    }
    return rect_f::zero_one;
}

// system
void update_layout(ecs::EntityApi e) {
    const auto& l = e.get<LayoutRect>();
    auto top_rect = find_parent_layout_rect(e, l.doSafeInsets);
    if (top_rect.empty()) {
        return;
    }
    auto& transform = e.get<Transform2D>();
    if ((l.fill_x || l.fill_y) && e.has<Display2D>()) {
        auto& display = e.get<Display2D>();
        auto* quad = display.tryGet<Quad2D>();
        if (quad) {
            if (l.fill_x) {
                quad->rect.x = top_rect.x;
                quad->rect.width = top_rect.width;
            }
            if (l.fill_y) {
                quad->rect.y = top_rect.y;
                quad->rect.height = top_rect.height;
            }
        } else if (display.is<Sprite2D>()) {
            auto& sprite = display.get<Sprite2D>();
            auto bounds = sprite.getBounds();
            if (!bounds.empty() && (l.fill_x || l.fill_y)) {
                auto pos = transform.getPosition();
                auto scale = transform.getScale();
                if (l.fill_x) {
                    pos.x = top_rect.x + l.fill_extra.x;
                    scale.x = (top_rect.width + l.fill_extra.width) / bounds.width;
                }
                if (l.fill_y) {
                    pos.y = top_rect.y + l.fill_extra.y;
                    scale.y = (top_rect.height + l.fill_extra.height) / bounds.height;
                }
                transform.setScale(scale);
                transform.setPosition(pos);
            }
        }
    }

    if (l.align_x) {
        transform.setX(top_rect.x + l.x.y + l.x.x * top_rect.width);
    }

    if (l.align_y) {
        transform.setY(top_rect.y + l.y.y + l.y.x * top_rect.height);
    }
}

void LayoutRect::updateAll() {
    for (auto e : ecs::view<LayoutRect, Transform2D>()) {
        update_layout(e);
    }
}

/**** wrapper ****/
rect_f LayoutRect::DesignCanvasRect{0.0f, 0.0f, 1.0f, 1.0f};

}