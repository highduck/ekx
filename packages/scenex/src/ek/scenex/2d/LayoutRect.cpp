#include "LayoutRect.hpp"
#include "Display2D.hpp"
#include <ek/scenex/base/Node.hpp>

namespace ek {

rect_t find_parent_layout_rect(entity_t e, bool safe) {
    auto it = get_parent(e);
    while (it.id) {
        const auto* layout = ecs::try_get<LayoutRect>(it);
        if (layout && !rect_is_empty(layout->rect)) {
            return safe ? layout->safeRect : layout->rect;
        }
        it = get_parent(it);
    }
    return rect_01();
}

// system
void update_layout(entity_t e) {
    const auto& l = ecs::get<LayoutRect>(e);
    auto top_rect = find_parent_layout_rect(e, l.doSafeInsets);
    if (UNLIKELY(rect_is_empty(top_rect))) {
        return;
    }
    auto* transform = ecs::try_get<Transform2D>(e);
    if(transform) {
        auto* display = ecs::try_get<Display2D>(e);
        if ((l.fill_x || l.fill_y) && display) {
            auto* quad = ecs::try_get<Quad2D>(e);
            if (quad) {
                if (l.fill_x) {
                    quad->rect.x = top_rect.x;
                    quad->rect.w = top_rect.w;
                }
                if (l.fill_y) {
                    quad->rect.y = top_rect.y;
                    quad->rect.h = top_rect.h;
                }
            } else if (ecs::has<Sprite2D>(e)) {
                auto bounds = sprite2d_get_bounds(e);
                if (!rect_is_empty(bounds) && (l.fill_x || l.fill_y)) {
                    auto pos = transform->getPosition();
                    auto scale = transform->getScale();
                    if (l.fill_x) {
                        pos.x = top_rect.x + l.fill_extra.x;
                        scale.x = (top_rect.w + l.fill_extra.w) / bounds.w;
                    }
                    if (l.fill_y) {
                        pos.y = top_rect.y + l.fill_extra.y;
                        scale.y = (top_rect.h + l.fill_extra.h) / bounds.h;
                    }
                    transform->set_scale(scale);
                    transform->set_position(pos);
                }
            }
        }

        if (l.align_x) {
            transform->setX(top_rect.x + l.x.y + l.x.x * top_rect.w);
        }

        if (l.align_y) {
            transform->setY(top_rect.y + l.y.y + l.y.x * top_rect.h);
        }
    }
}

void LayoutRect::updateAll() {
    foreach_type(ecs::type<LayoutRect>(), [](component_handle_t handle) {
        entity_t e = get_entity(ecs::type<LayoutRect>(), handle);
        update_layout(e);
    });
}

}