#include "editor.hpp"

#include <scenex/simple_audio_manager.h>
#include <scenex/asset2/asset_manager.hpp>
#include <ek/editor/assets/flash_asset.hpp>
#include <ek/editor/assets/audio_asset.hpp>
#include <ek/editor/assets/freetype_asset.hpp>
#include <ek/editor/assets/editor_temp_atlas.hpp>
#include <ek/editor/gui/editor_widgets.hpp>
#include <ek/editor/gui/gui_dev_mode.h>
#include <ek/editor/assets/program_asset.hpp>
#include <ek/editor/assets/texture_asset.hpp>
#include <ek/editor/assets/model_asset.hpp>

namespace ek {

editor_context_t::editor_context_t(scenex::basic_application& app)
        : app_{&app} {
    assets.add_resolver(new scenex::editor_asset_resolver_t<texture_asset_t>("texture"));
    assets.add_resolver(new scenex::editor_asset_resolver_t<program_asset_t>("program"));
    assets.add_resolver(new scenex::editor_asset_resolver_t<freetype_asset_t>("freetype"));
    assets.add_resolver(new scenex::editor_asset_resolver_t<flash_asset_t>("flash"));
    assets.add_resolver(new scenex::editor_asset_resolver_t<model_asset_t>("model"));
    assets.add_resolver(new scenex::editor_asset_resolver_t<audio_asset_t>("audio"));

    ek::gApp.listen(&imgui);
    app.hook_on_preload.add([this]() {
        assets.set_scale_factor(app_->scale_factor);
        scan_assets_folder(assets);
    });
    t2 = app.hook_on_update.add([this](float dt) {
        assets.set_scale_factor(app_->scale_factor);
        imgui.begin_frame(dt);
        gui_dev_mode();
        gui_asset_project(assets);
    });
    t1 = app.hook_on_draw_frame.add([this]() {
        imgui.end_frame();
    });
    t3 = app.hook_on_render_frame.add([this]() {

    });
}

editor_context_t::~editor_context_t() {
    app_->hook_on_update.remove(t2);
    app_->hook_on_draw_frame.remove(t1);
    app_->hook_on_render_frame.remove(t3);
}

}