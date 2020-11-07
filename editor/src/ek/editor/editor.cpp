#include "editor.hpp"

#include <ek/editor/gui/editor_widgets.hpp>
#include <ek/editor/gui/gui_dev_mode.hpp>

namespace ek {

using app::g_app;

editor_context_t::editor_context_t(basic_application& app)
        : app_{&app} {

    g_app.on_frame_completed += [&] { gui_.on_frame_completed(); };
    g_app.on_event += [&](auto e) { this->on_event(e); };

    app.hook_on_preload.add([this]() {
        project.update_scale_factor(app_->scale_factor);
        project.populate();
    });
    t2 = app.hook_on_update.add([this](float dt) {
        project.update_scale_factor(app_->scale_factor);
        gui_.begin_frame(dt);
        if (enabled_) {
            gui_dev_mode();
            gui_asset_project(project);
        }
    });
    t1 = app.hook_on_draw_frame.add([this]() {
        gui_.end_frame();
    });
    t3 = app.hook_on_render_frame.add([this]() {

    });
}

editor_context_t::~editor_context_t() {
    app_->hook_on_update.remove(t2);
    app_->hook_on_draw_frame.remove(t1);
    app_->hook_on_render_frame.remove(t3);
}

void editor_context_t::on_event(const app::event_t& event) {
    if (event.type == app::event_type::key_down
        && event.code == app::key_code::A
        && event.ctrl && event.shift) {
        enabled_ = !enabled_;
    }
    gui_.on_event(event);
}

}