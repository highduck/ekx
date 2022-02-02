#include "LayoutRect.hpp"
#include "Display2D.hpp"
#include <ek/scenex/base/Node.hpp>

namespace ek {

rect_t find_parent_layout_rect(ecs::EntityApi e, bool safe) {
    auto it = e.get<Node>().parent;
    while (it) {
        const auto* layout = it.tryGet<LayoutRect>();
        if (layout && !rect_is_empty(layout->rect)) {
            return safe ? layout->safeRect : layout->rect;
        }
        it = it.get<Node>().parent;
    }
    return rect_01();
}

// system
void update_layout(ecs::EntityApi e) {
    const auto& l = e.get<LayoutRect>();
    auto top_rect = find_parent_layout_rect(e, l.doSafeInsets);
    if (rect_is_empty(top_rect)) {
        return;
    }
    auto& transform = e.get<Transform2D>();
    if ((l.fill_x || l.fill_y) && e.has<Display2D>()) {
        auto& display = e.get<Display2D>();
        auto* quad = e.tryGet<Quad2D>();
        if (quad) {
            if (l.fill_x) {
                quad->rect.x = top_rect.x;
                quad->rect.w = top_rect.w;
            }
            if (l.fill_y) {
                quad->rect.y = top_rect.y;
                quad->rect.h = top_rect.h;
            }
        } else if (e.has<Sprite2D>()) {
            auto bounds = sprite2d_get_bounds(e.index);
            if (!rect_is_empty(bounds) && (l.fill_x || l.fill_y)) {
                auto pos = transform.getPosition();
                auto scale = transform.getScale();
                if (l.fill_x) {
                    pos.x = top_rect.x + l.fill_extra.x;
                    scale.x = (top_rect.w + l.fill_extra.w) / bounds.w;
                }
                if (l.fill_y) {
                    pos.y = top_rect.y + l.fill_extra.y;
                    scale.y = (top_rect.h + l.fill_extra.h) / bounds.h;
                }
                transform.setScale(scale);
                transform.setPosition(pos);
            }
        }
    }

    if (l.align_x) {
        transform.setX(top_rect.x + l.x.y + l.x.x * top_rect.w);
    }

    if (l.align_y) {
        transform.setY(top_rect.y + l.y.y + l.y.x * top_rect.h);
    }
}

void LayoutRect::updateAll() {
    for (auto e : ecs::view<LayoutRect, Transform2D>()) {
        update_layout(e);
    }
}

}