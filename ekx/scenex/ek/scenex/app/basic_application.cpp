#include "basic_application.hpp"

#include "input_controller.hpp"

#include <ek/imaging/ImageSubSystem.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/util/logger.hpp>
#include <ek/app/device.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/scenex/asset2/builtin_assets.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/3d/RenderSystem3D.hpp>
#include <ek/scenex/3d/Transform3D.hpp>
#include <ek/scenex/3d/Camera3D.hpp>
#include <ek/scenex/3d/Light3D.hpp>
#include <ek/scenex/3d/StaticMesh.hpp>

#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/UglyFilter2D.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/goodies/Shake.hpp>
#include <ek/goodies/PopupManager.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/goodies/GameScreen.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <Tracy.hpp>

namespace ek {

using namespace ek::app;

// additional insets can be changed manually: l, t, r, b
// to invalidate after change: setScreenRects(basic_application::root);
static const float4 DebugAdditionalInsets = float4::zero;
//static const float4 DebugAdditionalInsets{10, 20, 30, 40};

void updateRootViewport(Viewport& vp) {
    const auto screenSize = app::g_app.drawable_size;
    const auto screenDpiScale = (float)app::g_app.content_scale;

    float4 insets;
    getScreenInsets(insets.data());
    insets += DebugAdditionalInsets;

    vp.input.fullRect = rect_f{0.0f, 0.0f, (float)screenSize.x, (float)screenSize.y};
    vp.input.dpiScale = screenDpiScale;
    vp.input.safeRect = vp.input.fullRect;
    vp.input.safeRect.x += insets.x;
    vp.input.safeRect.y += insets.y;
    vp.input.safeRect.width -= insets.x + insets.z;
    vp.input.safeRect.height -= insets.y + insets.w;
}


void drawPreloader(float progress);

basic_application::basic_application() {
    tracy::SetThreadName("Main");

    graphics::initialize();
    audio::initialize();

    asset_manager_ = new asset_manager_t{};
}

basic_application::~basic_application() {

    ecs::the_world.shutdown();

    draw2d::state.shutdown();

    delete asset_manager_;

    audio::shutdown();
    graphics::shutdown();

    service_locator_instance<basic_application>::shutdown();
}

void basic_application::initialize() {
    ZoneScopedN("initialize");
    draw2d::state.initialize();

    // init default empty sprite data
    {
        auto* spr = new Sprite();
        spr->texture.setID("empty");
        Res<Sprite>{"empty"}.reset(spr);
    }

    //// basic scene
    using ecs::the_world;

    the_world.initialize();
    the_world.registerComponent<Node>(512);
    the_world.registerComponent<NodeName>(512);

    the_world.registerComponent<Camera2D>();
    the_world.registerComponent<Transform2D>(512);
    the_world.registerComponent<WorldTransform2D>(512);
    the_world.registerComponent<Display2D>(256);
    the_world.registerComponent<Bounds2D>();

    the_world.registerComponent<Transform3D>();
    the_world.registerComponent<Camera3D>();
    the_world.registerComponent<Light3D>();
    the_world.registerComponent<MeshRenderer>();

    the_world.registerComponent<MovieClip>();
    the_world.registerComponent<MovieClipTargetIndex>();

    the_world.registerComponent<LayoutRect>();
    the_world.registerComponent<Viewport>();
    the_world.registerComponent<Button>();
    the_world.registerComponent<Interactive>();
    the_world.registerComponent<Tween>();
    the_world.registerComponent<Shake>();
    the_world.registerComponent<ScriptHolder>();
    the_world.registerComponent<BubbleText>();
    the_world.registerComponent<PopupManager>();
    the_world.registerComponent<close_timeout>();
    the_world.registerComponent<GameScreen>();
    the_world.registerComponent<Trail2D>();
    the_world.registerComponent<DestroyTimer>();
    the_world.registerComponent<NodeEventHandler>();
    the_world.registerComponent<ParticleEmitter2D>(4);
    the_world.registerComponent<ParticleLayer2D>();
    the_world.registerComponent<UglyFilter2D>();

    root = createNode2D("root");
    root.assign<Viewport>(AppResolution.x, AppResolution.y);
    root.assign<LayoutRect>();
    root.assign<NodeEventHandler>();
    updateRootViewport(root.get<Viewport>());
    Viewport::updateAll();
    scale_factor = root.get<Viewport>().output.scale;

    auto& im = service_locator_instance<InteractionSystem>::init(root);
    service_locator_instance<input_controller>::init(im);
    service_locator_instance<AudioManager>::init();

    auto camera = createNode2D("camera");
    auto& defaultCamera = camera.assign<Camera2D>(root);
    defaultCamera.order = 1;
    defaultCamera.viewportNode = ecs::EntityRef{root};
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
    tracy::SetThreadName("Render");
    FrameMark;
    ZoneScoped;
    timer_t timer{};
    scale_factor = root.get<Viewport>().output.scale;
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
            .value = {0.0f, 0.0f, 0.0f, 1.0f}
    };
    if (r3d) {
        pass_action.depth.action = SG_ACTION_CLEAR;
        pass_action.depth.value = 100000.0f;
    }

    if (!started_ && rootAssetObject) {
        rootAssetObject->poll();
    }

    auto fbWidth = static_cast<int>(g_app.drawable_size.x);
    auto fbHeight = static_cast<int>(g_app.drawable_size.y);
    if(fbWidth > 0 && fbHeight > 0) {
        sg_begin_default_pass(&pass_action, fbWidth, fbHeight);

        if (r3d) {
            r3d->render();
        }

        render_frame();

        draw2d::begin({0, 0, (float)fbWidth, (float)fbHeight});

        if (!started_ && rootAssetObject) {
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

    }
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
    (void)dt;
}

void basic_application::render_frame() {
    scene_render(root);
}

void basic_application::on_frame_end() {
    ZoneScoped;
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
    ZoneScoped;
    timer_t timer{};
    if (event.type == event_type::app_resize) {
        updateRootViewport(root.get<Viewport>());
    }
    profiler.addTime("EVENTS", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
}

void drawPreloader(float progress) {
    draw2d::state.setEmptyTexture();
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
