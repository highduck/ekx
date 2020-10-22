#include "basic_application.hpp"

#include "input_controller.hpp"
#include "app_utils.hpp"

#include <ek/scenex/interactive_manager.hpp>
#include <ek/scenex/simple_audio_manager.hpp>
#include <ek/scenex/components/canvas.hpp>
#include <ek/scenex/systems/layout_system.hpp>
#include <ek/scenex/systems/canvas_system.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/ext/game_center/game_center.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/app/device.hpp>

namespace ek {

using ecs::world;
using ecs::entity;
using namespace ek::app;

basic_application::basic_application()
        : base_resolution{static_cast<float>(g_app.window_cfg.size.x),
                          static_cast<float>(g_app.window_cfg.size.y)} {

    assert_created_once<basic_application>();

    graphics::init();
    audio::init();

    asset_manager_ = new asset_manager_t{};

    ecxx::set_world(&w);

    game_services_init();
    // set callbacks before ads initialization (TODO)
    // BasicGameUtility::init();
}

basic_application::~basic_application() {
    delete asset_manager_;
}

void basic_application::initialize() {
    create_builtin();

    //// basic scene
    root = create_node_2d("root");
    setScreenRects(root);

    auto& im = service_locator_instance<interactive_manager>::init();
    service_locator_instance<input_controller>::init(im);
    service_locator_instance<simple_audio_manager>::init();

    im.set_entity(root);

    game = create_node_2d("game");
    ecs::assign<canvas_t>(game, base_resolution.x, base_resolution.y);
    ecs::assign<layout_t>(game);
    append(root, game);
    scale_factor = update_canvas(game);

    layout_wrapper::designCanvasRect = {0.0f, 0.0f, base_resolution.x, base_resolution.y};
}

void basic_application::preload() {
    hook_on_preload();

    EK_DEBUG("Loading scale: %0.3f", scale_factor);

    asset_manager_->set_scale_factor(scale_factor);
    asset_manager_->add_resolver(new builtin_asset_resolver_t());
    // built-in
//    asset_manager_->add_from_type("font", "mini");
//    asset_manager_->add_from_type("atlas", "mini");
//    asset_manager_->add_from_type("program", "2d");
    preload_root_assets_pack();
}

void basic_application::on_draw_frame() {
    scale_factor = ecs::get<canvas_t>(game).scale;
    asset_manager_->set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const double dt = std::min(frame_timer.update(), 0.3);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;

    hook_on_update(static_cast<float>(dt));
    update_frame(static_cast<float>(dt));

    graphics::begin();
    graphics::viewport();
    //graphics.set_scissors();

    if (clear_color_enabled) {
        graphics::clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    }

    draw2d::begin(0, 0,
                  static_cast<int>(g_app.drawable_size.x),
                  static_cast<int>(g_app.drawable_size.y));

    render_frame();

    hook_on_render_frame();

    on_frame_end();

    draw2d::end();

    hook_on_draw_frame();
    /** base app END **/

    if (!started_ && asset_manager_->is_assets_ready()) {
        start_game();
        started_ = true;
    }
}

void basic_application::update_frame(float dt) {
}

void basic_application::render_frame() {
    scene_render(root);
}

void basic_application::on_frame_end() {
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

void basic_application::on_event(const event_t& event) {
    if(event.type == event_type::app_resize) {
        setScreenRects(root);
    }
}


//void basic_application::on_event(const event_t& event) {}

}