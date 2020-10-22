#include "layout.hpp"
#include <ecxx/ecxx.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/app/device.hpp>
#include <ek/app/app.hpp>

namespace ek {

rect_f find_parent_layout_rect(ecs::entity e, bool safe) {
    auto it = e;
    while (it) {
        if(ecs::has<layout_t>(it)) {
            const auto& layout = ecs::get<layout_t>(it);
            if (!layout.rect.empty()) {
                return safe ? layout.safeRect : layout.rect;
            }
        }
        it = ecs::get<node_t>(it).parent;
    }
    return rect_f::zero_one;
}

rect_f get_ancestors_rect(ecs::entity e) {
    rect_f rect{0.0f, 0.0f, 1.0f, 1.0f};
    auto it = ecs::get<node_t>(e).parent;
    while (it) {
        if(ecs::has<layout_t>(it)) {
            rect = ecs::get<layout_t>(it).rect;
            if (!rect.empty()) {
                break;
            }
        }
        it = ecs::get<node_t>(it).parent;
    }
    return rect;
}

void setScreenRects(ecs::entity root) {
    const auto screen_size = app::g_app.drawable_size;
    auto& rootLayout = ecs::get_or_create<layout_t>(root);
    rootLayout.rect.set(0.0f, 0.0f, screen_size.x, screen_size.y);
    auto insets = get_screen_insets();
//    insets[0] = 100;
//    insets[1] = 200;
//    insets[2] = 100;
//    insets[3] = 200;
    rootLayout.safeRect.set(
            rootLayout.rect.x + insets[0],
            rootLayout.rect.y + insets[1],
            rootLayout.rect.width - insets[0] - insets[2],
            rootLayout.rect.height - insets[1] - insets[3]
    );
}

}