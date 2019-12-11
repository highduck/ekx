#include "layout_system.hpp"

#include <ek/scenex/components/layout.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/utility/scene_management.hpp>

namespace ek {

void update_layout(ecs::entity e) {
    auto top_rect = find_root_rect(e);
    if (top_rect.empty()) {
        return;
    }

    const auto& l = ecs::get<layout_t>(e);
    auto& transform = ecs::get<transform_2d>(e);
    if ((l.fill_x || l.fill_y) && ecs::has<display_2d>(e)) {
        auto& drawable = ecs::get<display_2d>(e).drawable;
        if (drawable) {
            if (drawable->get_type_id() == drawable_quad::type_id) {
                auto* quad = static_cast<drawable_quad*>(drawable.get());
                if (l.fill_x) {
                    quad->rect.x = top_rect.x;
                    quad->rect.width = top_rect.width;
                }
                if (l.fill_y) {
                    quad->rect.y = top_rect.y;
                    quad->rect.height = top_rect.height;
                }
            } else if (drawable->get_type_id() == drawable_sprite::type_id) {
                auto* sprite = static_cast<drawable_sprite*>(drawable.get());
                auto bounds = sprite->get_bounds();
                if (!bounds.empty()) {
                    if (l.fill_x) {
                        transform.matrix.tx = top_rect.x + l.fill_extra.x;
                        transform.scale.x = (top_rect.width + l.fill_extra.width) / bounds.width;
                    }
                    if (l.fill_y) {
                        transform.matrix.ty = top_rect.y + l.fill_extra.y;
                        transform.scale.y = (top_rect.height + l.fill_extra.height) / bounds.height;
                    }
                }
            }
        }
    }

    if (l.align_x) {
        transform.matrix.tx = top_rect.x + l.x.y + l.x.x * top_rect.width;
    }

    if (l.align_y) {
        transform.matrix.ty = top_rect.y + l.y.y + l.y.x * top_rect.height;
    }
}

void update_layout() {
    for (auto e : ecs::view<layout_t, transform_2d>()) {
        update_layout(e);
    }
}

/**** wrapper ****/
rect_f layout_wrapper::space{0.0f, 0.0f, 1.0f, 1.0f};


}