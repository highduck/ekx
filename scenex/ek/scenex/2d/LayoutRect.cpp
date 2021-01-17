#include "LayoutRect.hpp"
#include "Display2D.hpp"
#include <ek/scenex/base/Node.hpp>
#include <ek/app/device.hpp>
#include <ek/app/app.hpp>

namespace ek {

float4 LayoutRect::AdditionalInsets = float4::zero;

rect_f find_parent_layout_rect(ecs::entity e, bool safe) {
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

void updateScreenRect(ecs::entity root) {
    const auto screen_size = app::g_app.drawable_size;
    auto& rootLayout = root.get_or_create<LayoutRect>();
    rootLayout.rect.set(0.0f, 0.0f, screen_size.x, screen_size.y);

    auto insets = LayoutRect::AdditionalInsets + get_screen_insets();
    rootLayout.safeRect = rootLayout.rect;
    rootLayout.safeRect.x += insets.x;
    rootLayout.safeRect.y += insets.y;
    rootLayout.safeRect.width -= insets.x + insets.z;
    rootLayout.safeRect.height -= insets.y + insets.w;
}

// system


void update_layout(ecs::entity e) {
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

void LayoutRect::updateAll() {
    for (auto e : ecs::view<LayoutRect, Transform2D>()) {
        update_layout(e);
    }
}

/**** wrapper ****/
rect_f layout_wrapper::designCanvasRect{0.0f, 0.0f, 1.0f, 1.0f};

}