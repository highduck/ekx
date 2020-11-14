#include "editor.hpp"

#include <ek/editor/gui/editor_widgets.hpp>
#include <ek/editor/gui/gui_dev_mode.hpp>
#include <ek/scenex/app/basic_application.hpp>

namespace ek {

using app::g_app;

Editor::Editor(basic_application& app)
        : app_{&app} {

    g_app.on_frame_completed += [&] { gui_.on_frame_completed(); };
    g_app.on_event += [&](auto e) { this->on_event(e); };

    app.hook_on_preload.add([this]() {
        project.update_scale_factor(app_->scale_factor, false);
        project.populate();
    });
    t2 = app.hook_on_update.add([this](float dt) {
        project.update_scale_factor(app_->scale_factor, settings.notifyAssetsOnScaleFactorChanged);
        gui_.begin_frame(dt);
        if (settings.showEditor) {
            gui_dev_mode(*this);
            gui_asset_project(project);
        }
    });
    t1 = app.hook_on_draw_frame.add([this]() {
        gui_.end_frame();
    });
    t3 = app.hook_on_render_frame.add([this]() {

    });

    app.preloadOnStart = false;
}

Editor::~Editor() {
    app_->hook_on_update.remove(t2);
    app_->hook_on_draw_frame.remove(t1);
    app_->hook_on_render_frame.remove(t3);
}

void Editor::on_event(const app::event_t& event) {
    if (event.type == app::event_type::key_down
        && event.code == app::key_code::A
        && event.ctrl && event.shift) {
        settings.showEditor = !settings.showEditor;
    }
    gui_.on_event(event);
}

void Editor::initialize() {
    service_locator_instance<Editor>::init(resolve<basic_application>());
}

}