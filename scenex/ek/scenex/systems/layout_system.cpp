#include "layout_system.hpp"

#include <ek/scenex/components/layout.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/display_2d.hpp>

namespace ek {

void update_layout(ecs::entity e) {
    const auto& l = ecs::get<layout_t>(e);
    auto top_rect = find_parent_layout_rect(e, l.doSafeInsets);
    if (top_rect.empty()) {
        return;
    }
    auto& transform = ecs::get<transform_2d>(e);
    if ((l.fill_x || l.fill_y) && ecs::has<display_2d>(e)) {
        auto& display = ecs::get<display_2d>(e);
        if (display.is<drawable_quad>()) {
            auto* quad = display.get<drawable_quad>();
            if (l.fill_x) {
                quad->rect.x = top_rect.x;
                quad->rect.width = top_rect.width;
            }
            if (l.fill_y) {
                quad->rect.y = top_rect.y;
                quad->rect.height = top_rect.height;
            }
        } else if (display.is<drawable_sprite>()) {
            auto* sprite = display.get<drawable_sprite>();
            auto bounds = sprite->get_bounds();
            if (!bounds.empty()) {
                if (l.fill_x) {
                    transform.position.x = top_rect.x + l.fill_extra.x;
                    transform.scale.x = (top_rect.width + l.fill_extra.width) / bounds.width;
                }
                if (l.fill_y) {
                    transform.position.y = top_rect.y + l.fill_extra.y;
                    transform.scale.y = (top_rect.height + l.fill_extra.height) / bounds.height;
                }
            }
        }
    }

    if (l.align_x) {
        transform.position.x = top_rect.x + l.x.y + l.x.x * top_rect.width;
    }

    if (l.align_y) {
        transform.position.y = top_rect.y + l.y.y + l.y.x * top_rect.height;
    }
}

void update_layout() {
    for (auto e : ecs::view<layout_t, transform_2d>()) {
        update_layout(e);
    }
}

/**** wrapper ****/
rect_f layout_wrapper::designCanvasRect{0.0f, 0.0f, 1.0f, 1.0f};


}