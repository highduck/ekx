#include "layout.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/components/Node.hpp>
#include <ek/app/device.hpp>
#include <ek/app/app.hpp>

namespace ek {

float4 layout_t::AdditionalInsets = float4::zero;

rect_f find_parent_layout_rect(ecs::entity e, bool safe) {
    auto it = e;
    while (it) {
        if (ecs::has<layout_t>(it)) {
            const auto& layout = ecs::get<layout_t>(it);
            if (!layout.rect.empty()) {
                return safe ? layout.safeRect : layout.rect;
            }
        }
        it = ecs::get<Node>(it).parent;
    }
    return rect_f::zero_one;
}

rect_f get_ancestors_rect(ecs::entity e) {
    rect_f rect{0.0f, 0.0f, 1.0f, 1.0f};
    auto it = ecs::get<Node>(e).parent;
    while (it) {
        if (ecs::has<layout_t>(it)) {
            rect = ecs::get<layout_t>(it).rect;
            if (!rect.empty()) {
                break;
            }
        }
        it = ecs::get<Node>(it).parent;
    }
    return rect;
}

void updateScreenRect(ecs::entity root) {
    const auto screen_size = app::g_app.drawable_size;
    auto& rootLayout = ecs::get_or_create<layout_t>(root);
    rootLayout.rect.set(0.0f, 0.0f, screen_size.x, screen_size.y);

    auto insets = layout_t::AdditionalInsets + get_screen_insets();
    rootLayout.safeRect = rootLayout.rect;
    rootLayout.safeRect.x += insets.x;
    rootLayout.safeRect.y += insets.y;
    rootLayout.safeRect.width -= insets.x + insets.z;
    rootLayout.safeRect.height -= insets.y + insets.w;
}

}