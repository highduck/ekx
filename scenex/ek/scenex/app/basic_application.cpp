#include "basic_application.hpp"

#include "input_controller.hpp"
#include "builtin_resources.hpp"

#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/Canvas.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/ext/game_center/game_center.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/Camera2D.hpp>

namespace ek {

using ecs::world;
using ecs::entity;
using namespace ek::app;

basic_application::basic_application() {

    assert_created_once<basic_application>();

    graphics::init();
    audio::init();

    asset_manager_ = new asset_manager_t{};

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
    updateScreenRect(root);

    auto& defaultCamera = root.assign<Camera2D>(root);
    defaultCamera.order = 1;
    Camera2D::Main = root;

    auto& im = service_locator_instance<InteractionSystem>::init(root);
    service_locator_instance<input_controller>::init(im);
    service_locator_instance<AudioManager>::init();

    game = create_node_2d("game");
    ecs::assign<Canvas>(game, AppResolution.x, AppResolution.y);
    ecs::assign<LayoutRect>(game);
    append(root, game);
    Canvas::updateAll();
    scale_factor = game.get<Canvas>().scale;

    layout_wrapper::designCanvasRect = {float2::zero, AppResolution};
}

void basic_application::preload() {
    EK_DEBUG("Loading scale: %0.3f", scale_factor);
    asset_manager_->set_scale_factor(scale_factor);

    hook_on_preload();

    asset_manager_->add_resolver(new builtin_asset_resolver_t());
    // built-in
//    asset_manager_->add_from_type("font", "mini");
//    asset_manager_->add_from_type("atlas", "mini");
//    asset_manager_->add_from_type("program", "2d");
    if (preloadOnStart) {
        preload_root_assets_pack();
    }
}

void basic_application::on_draw_frame() {
    timer_t timer{};
    scale_factor = ecs::get<Canvas>(game).scale;
    asset_manager_->set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const double dt = std::min(frame_timer.update(), 0.3);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;

    hook_on_update(static_cast<float>(dt));
    update_frame(static_cast<float>(dt));

    profiler.addTime("UPDATE", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    graphics::begin();
    graphics::viewport();

    draw2d::begin(0, 0,
                  static_cast<int>(g_app.drawable_size.x),
                  static_cast<int>(g_app.drawable_size.y));

    render_frame();

    hook_on_render_frame();

    profiler.addTime("RENDER", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    on_frame_end();

    profiler.addTime("PROFILER", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    draw2d::end();

    hook_on_draw_frame();
    /** base app END **/

    if (!started_ && asset_manager_->is_assets_ready()) {
        start_game();
        started_ = true;
    }

    profiler.addTime("END", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    profiler.update(frame_timer.delta_time());
}

void basic_application::update_frame(float dt) {
}

void basic_application::render_frame() {
    scene_render(root);
}

void basic_application::on_frame_end() {
    //profiler.update(frame_timer.delta_time());
    profiler.draw();
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
    timer_t timer{};
    if (event.type == event_type::app_resize) {
        updateScreenRect(root);
    }
    profiler.addTime("EVENTS", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
}


//void basic_application::on_event(const event_t& event) {}

}
