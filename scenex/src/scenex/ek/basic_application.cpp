#include "basic_application.h"

#include "input_controller.h"
#include "app_utils.hpp"

#include <scenex/interactive_manager.h>
#include <scenex/simple_audio_manager.h>
#include <scenex/components/canvas.h>
#include <scenex/systems/layout_system.h>
#include <scenex/systems/canvas_system.h>
#include <scenex/systems/main_flow.h>
#include <scenex/utility/scene_management.h>
#include <platform/game_center.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/common_macro.hpp>
#include <scenex/asset2/asset_manager.hpp>
#include <scenex/asset2/builtin_assets.hpp>

namespace scenex {

using ecs::world;
using ecs::entity;
using ek::service_locator_instance;
using ek::g_app;
using ek::resolve;

basic_application::basic_application()
        : base_resolution{static_cast<float>(g_app.creation_config.size.x),
                          static_cast<float>(g_app.creation_config.size.y)} {

    ek::assert_created_once<basic_application>();

    asset_manager_ = new asset_manager_t{};

    ecxx::set_world(&w);

    ek::game_services_init();
    // set callbacks before ads initialization (TODO)
    //ek::BasicGameUtility::init();
}

basic_application::~basic_application() {
    delete asset_manager_;
}

void basic_application::initialize() {
    base_app_t::initialize();

    create_builtin();

    //// basic scene
    root = create_node_2d("root");
    const auto screen_size = g_app.drawable_size;
    ecs::get<scenex::transform_2d>(root).rect.set(0.0f, 0.0f, screen_size.x, screen_size.y);

    auto& im = service_locator_instance<interactive_manager>::init();
    //auto& input =
    service_locator_instance<input_controller>::init(im);
    //auto& audio =
    service_locator_instance<simple_audio_manager>::init();

    im.set_entity(root);

    game = create_node_2d("game");
    ecs::assign<canvas_t>(game, base_resolution.x, base_resolution.y);

    scenex::append(root, game);
    scale_factor = update_canvas(game);

    layout_wrapper::space = {0.0f, 0.0f, base_resolution.x, base_resolution.y};
}

void basic_application::preload() {
    base_app_t::preload();
    hook_on_preload();

    EK_DEBUG("Loading scale: %0.3f", scale_factor);

    //using namespace scenex::assets;
    asset_manager_->set_scale_factor(scale_factor);
    asset_manager_->add_resolver(new builtin_asset_resolver_t());
    // built-in
//    asset_manager_->add_from_type("font", "mini");
//    asset_manager_->add_from_type("atlas", "mini");
//    asset_manager_->add_from_type("program", "2d");
    preload_root_assets_pack();
}

void basic_application::on_draw_frame() {
    scale_factor = ecs::get<scenex::canvas_t>(game).scale;
    asset_manager_->set_scale_factor(scale_factor);

    base_app_t::on_draw_frame();

    if (!started_ && asset_manager_->is_assets_ready()) {
        start_game();
        started_ = true;
    }
}

void basic_application::update_frame(float dt) {
    base_app_t::update_frame(dt);
}

void basic_application::render_frame() {
    base_app_t::render_frame();
    scene_render(root);
}

void basic_application::on_frame_end() {
    base_app_t::on_frame_end();

#if !defined(NDEBUG) && !defined(__EMSCRIPTEN__)
    stats_.update();
    stats_.draw();
#endif
}

void basic_application::preload_root_assets_pack() {
    auto* asset_pack = asset_manager_->add_from_type("pack", "pack_meta");
    if (asset_pack) {
        asset_pack->load();
    }
}

void basic_application::start_game() {

}

}
