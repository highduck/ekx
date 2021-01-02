#include "basic_application.hpp"

#include "input_controller.hpp"

#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/Canvas.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/3d/RenderSystem3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>

namespace ek {

using ecs::world;
using ecs::entity;
using namespace ek::app;

void drawPreloader(float progress);

basic_application::basic_application() {

    assert_created_once<basic_application>();

    graphics::init();
    audio::init();

    asset_manager_ = new asset_manager_t{};
}

basic_application::~basic_application() {
    delete asset_manager_;
}

void basic_application::initialize() {
    draw2d::init();

    //// basic scene
    root = createNode2D("root");
    updateScreenRect(root);

    auto& im = service_locator_instance<InteractionSystem>::init(root);
    service_locator_instance<input_controller>::init(im);
    service_locator_instance<AudioManager>::init();

    game = createNode2D("game");
    ecs::assign<Canvas>(game, AppResolution.x, AppResolution.y);
    ecs::assign<LayoutRect>(game);
    append(root, game);
    Canvas::updateAll();
    scale_factor = game.get<Canvas>().scale;

    auto camera = createNode2D("camera");
    auto& defaultCamera = camera.assign<Camera2D>(game);
    defaultCamera.order = 1;
    Camera2D::Main = camera;
    append(root, camera);

    layout_wrapper::designCanvasRect = {float2::zero, AppResolution};
}

void basic_application::preload() {
    EK_DEBUG("Loading scale: %0.3f", scale_factor);
    asset_manager_->set_scale_factor(scale_factor);

    hook_on_preload();

    asset_manager_->add_resolver(new builtin_asset_resolver_t());
    if (preloadOnStart) {
        preload_root_assets_pack();
    }
}

void basic_application::on_draw_frame() {
    timer_t timer{};
    scale_factor = ecs::get<Canvas>(game).scale;
    asset_manager_->set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const float dt = std::min(static_cast<float>(frame_timer.update()), 0.3f);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;

    hook_on_update(dt);
    update_frame(dt);

    profiler.addTime("UPDATE", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    draw2d::beginNewFrame();

    onPreRender();

    auto* r3d = try_resolve<RenderSystem3D>();
    if (r3d) {
        Transform3D::updateAll();
        r3d->prepare();
        r3d->prerender();
    }

    static sg_pass_action pass_action{};
    pass_action.colors[0] = {
            .action = started_ ? SG_ACTION_DONTCARE : SG_ACTION_CLEAR,
            .val = {0.0f, 0.0f, 0.0f, 1.0f}
    };
    if (r3d) {
        pass_action.depth.action = SG_ACTION_CLEAR;
        pass_action.depth.val = 100000.0f;
    }

    auto fbWidth = static_cast<int>(g_app.drawable_size.x);
    auto fbHeight = static_cast<int>(g_app.drawable_size.y);
    sg_begin_default_pass(&pass_action, fbWidth, fbHeight);

    if (r3d) {
        r3d->render();
    }

    render_frame();

    draw2d::begin({0, 0, fbWidth, fbHeight});

    if (!started_ && rootAssetObject) {
        rootAssetObject->poll();
        drawPreloader(rootAssetObject->getProgress());
    }

    hook_on_render_frame();

    profiler.addTime("RENDER", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    on_frame_end();

    profiler.addTime("PROFILER", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    draw2d::end();
    draw2d::endFrame();

    hook_on_draw_frame();
    /** base app END **/
    sg_end_pass();
    sg_commit();

    if (!started_ && asset_manager_->is_assets_ready()) {
        EK_DEBUG << "Start Game";
        start_game();
        onStartHook();
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
    profiler.draw();
}

void basic_application::preload_root_assets_pack() {
    rootAssetObject = asset_manager_->add_from_type("pack", "pack_meta");
    if (rootAssetObject) {
        rootAssetObject->load();
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

void drawPreloader(float progress) {
    draw2d::current().set_empty_texture();
    auto pad = 40;
    auto w = g_app.drawable_size.x - pad * 2;
    auto h = 16;
    auto y = (g_app.drawable_size.y - h) / 2;

    auto color = 0x00000000_argb;
    color.af(1.0f - progress);
    draw2d::quad(0, 0, g_app.drawable_size.x, g_app.drawable_size.y, color);
    draw2d::quad(pad, y, w, h, 0x77000000_argb);
    draw2d::quad(pad + 4, y + 4, (w - 8) * progress, h - 8, 0x77FFFFFF_argb);
}

}
