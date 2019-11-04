#pragma once

#include <scenex/config/ecs.h>
#include <scenex/config.h>

#include <platform/Window.h>
#include <platform/Application.h>

namespace scenex {

class interactive_manager {
public:
    using app_event_t = ek::AppEvent;
    using cursor_t = ek::MouseCursor;
    using mouse_event_t = ek::MouseEvent;
    using touch_event_t = ek::TouchEvent;
    using key_event_t = ek::KeyEvent;

    bool debug_hit_enabled = false;
    float2 pointer_global_space = float2::zero;
    float2 pointer = float2::zero;
    bool pointer_down = false;

    interactive_manager() = default;

    ~interactive_manager() {
        assert(false);
    }

    void set_entity(ecs::entity e) {
        entity_ = e;
    }

    void update();

    cursor_t search_interactive_targets(ecs::entity node, std::vector<ecs::entity>& out_entities);

    void send_back_button();

    void handle_system_pause();

    void set_debug_hit(ecs::entity hit);

    void handle_mouse_event(const mouse_event_t& ev);

    void handle_touch_event(const touch_event_t& ev);
//
//    static void drawBorders(EventData ev) {
//        auto* v = ev.source->display;
//        if (v == nullptr) return;
//        auto& drawer = resolve<Drawer>();
//        Matrix matrix = drawer.matrix;
//        drawer.saveTransform();
//        drawer.colorMultiplier = 0xFF000000_argb;
//        drawer.colorOffset = 0x00FF7700_argb;
//        float a = 0.0f;
//        float d = 4.0f;
//        while (a < 2.0f * M_PI) {
//            drawer.matrix = Matrix{1.0f, 0.0f, 0.0f, 1.0f, d * cosf(a), d * sinf(a)};
//            drawer.concatMatrix(matrix);
//            v->draw();
//            a += M_PI * 2 / 8;
//        }
//
//        drawer.restoreTransform();
//    }
//
//    static void drawBounds(EventData ev) {
//        auto* font = asset_t<Font>::content("mini");
//        if (!font) return;
//
//        auto* node = ev.source;
//        if (node->display != nullptr) {
//            CDisplayUtil::draw_bounds(node->display->get_bounds(), 2.0f, 0xFFFFFFFF_argb);
//            auto path = CDisplayUtil::get_full_path(node);
//            auto pos = float2::zero;
//
//            for (auto const& p : path) {
//                font->draw(p, 10, pos + float2::one, 0xFF000000_argb, 10);
//                font->draw(p, 10, pos, 0xFFFFFFFF_argb, 10);
//                pos.x += 4;
//                pos.y += 15;
//            }
//        }
//    }

    [[nodiscard]] ecs::entity entity() const {
        return entity_;
    }

    void drag(ecs::entity entity) {
        drag_entity_ = entity;
    }

private:
    ecs::entity debug_hit_target_ = ecs::null;
    ecs::entity entity_ = ecs::null;

    std::vector<ecs::entity> last_targets_;
    std::vector<ecs::entity> targets_;

    bool mouse_active_ = false;
    uint64_t primary_touch_id_ = 0ull;
    float2 primary_touch_ = float2::zero;
    float2 primary_mouse_ = float2::zero;

    ecs::entity drag_entity_;
};

}


