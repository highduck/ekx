#include "basic_application.hpp"

#include "input_controller.hpp"

#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/SceneFactory.hpp>
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

#include "impl/GameDisplay_impl.hpp"

#ifdef EK_UITEST
#include "impl/uitest_impl.hpp"
#endif

namespace ek {

using namespace ek::app;

// additional insets can be changed manually: l, t, r, b
// to invalidate after change: setScreenRects(basic_application::root);
static const float4 DebugAdditionalInsets = float4::zero;
//static const float4 DebugAdditionalInsets{10, 20, 30, 40};

void updateRootViewport(Viewport& vp) {
    auto* baseApp = try_resolve<basic_application>();
    if (!baseApp) {
        return;
    }

    const auto& info = baseApp->display.info;
    const rect_f rc{0.0f, 0.0f, info.size.x, info.size.y};
    const float4 insets = info.insets + DebugAdditionalInsets;

    vp.input.fullRect = rc;
    vp.input.dpiScale = info.dpiScale;
    vp.input.safeRect = rc;
    vp.input.safeRect.x += insets.x;
    vp.input.safeRect.y += insets.y;
    vp.input.safeRect.width -= insets.x + insets.z;
    vp.input.safeRect.height -= insets.y + insets.w;
}

void logDisplayInfo() {
#ifndef NDEBUG
    const auto sz = app::g_app.drawable_size;
    float insets[4]{};
    getScreenInsets(insets);
    EK_INFO << "Display: " << sz.x << " x " << sz.y << " [" << insets[0] << ", " << insets[1] << ", " << insets[2] << ", " << insets[3] << "]";
#endif
}

void drawPreloader(float progress, float zoneWidth, float zoneHeight) {
    draw2d::state.setEmptyTexture();
    auto pad = 40.0f;
    auto w = zoneWidth - pad * 2.0f;
    auto h = 16.0f;
    auto y = (zoneHeight - h) / 2.0f;

    draw2d::quad(pad, y, w, h, 0xFFFFFFFF_argb);
    draw2d::quad(pad + 2, y + 2, w - 4, h - 4, 0xFF000000_argb);
    draw2d::quad(pad + 4, y + 4, (w - 8) * progress, h - 8, 0xFFFFFFFF_argb);
}

basic_application::basic_application() {
    tracy::SetThreadName("Main");
#ifdef EK_UITEST
    uitest::initialize(this);
#endif
    asset_manager_ = new asset_manager_t{};
}

basic_application::~basic_application() {

    ecs::the_world.shutdown();

    draw2d::shutdown();

    delete asset_manager_;

    audio::shutdown();
    graphics::shutdown();

    service_locator_instance<basic_application>::shutdown();
}

void registerSceneXComponents() {
    //// basic scene
    using ecs::the_world;
    the_world.registerComponent<Node>(512);
    the_world.registerComponent<NodeName>(512);
    the_world.registerComponent<Transform2D>(512);
    the_world.registerComponent<WorldTransform2D>(512);
    the_world.registerComponent<Display2D>(256);
    the_world.registerComponent<Camera2D>();
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
    the_world.registerComponent<Shaker>();
    the_world.registerComponent<ScriptHolder>();
    the_world.registerComponent<Updater>();
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
}

void basic_application::initialize() {
    ZoneScopedN("initialize");

    EK_TRACE << "base application: initialize";
    // init default empty sprite data
    {
        auto* spr = new Sprite();
        spr->texture.setID("empty");
        Res<Sprite>{"empty"}.reset(spr);
    }

    logDisplayInfo();
    display.update();

    EK_TRACE << "base application: initialize scene root";
    root = createNode2D("root");
    root.assign<Viewport>(AppResolution.x, AppResolution.y);
    root.assign<LayoutRect>();
    root.assign<NodeEventHandler>();
    updateRootViewport(root.get<Viewport>());
    Viewport::updateAll();
    scale_factor = root.get<Viewport>().output.scale;

    EK_TRACE << "base application: initialize InteractionSystem";
    auto& im = service_locator_instance<InteractionSystem>::init(root);
    EK_TRACE << "base application: initialize input_controller";
    service_locator_instance<input_controller>::init(im);
    EK_TRACE << "base application: initialize AudioManager";
    service_locator_instance<AudioManager>::init();

    EK_TRACE << "base application: initialize Scene";
    auto camera = createNode2D("camera");
    auto& defaultCamera = camera.assign<Camera2D>(root);
    defaultCamera.order = 1;
    defaultCamera.viewportNode = ecs::EntityRef{root};
    Camera2D::Main = camera;
    append(root, camera);

    LayoutRect::DesignCanvasRect = {float2::zero, AppResolution};
}

void basic_application::preload() {
    EK_TRACE("base application: preloading, content scale: %0.3f", scale_factor);
    asset_manager_->set_scale_factor(scale_factor);

    hook_on_preload();

    asset_manager_->add_resolver(new builtin_asset_resolver_t());
    if (preloadOnStart) {
        preload_root_assets_pack();
    }
}

void basic_application::on_draw_frame() {
    ZoneScoped;
    timer_t timer{};
    display.update();
    scale_factor = root.get<Viewport>().output.scale;
    asset_manager_->set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const float dt = std::min(static_cast<float>(frame_timer.update()), 0.3f);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;
    doUpdateFrame(dt);
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
    pass_action.colors[0].action = started_ ? SG_ACTION_DONTCARE : SG_ACTION_CLEAR;
    const float4 fillColor{argb32_t{g_app.window_cfg.backgroundColor}};
    pass_action.colors[0].value.r = fillColor.x;
    pass_action.colors[0].value.g = fillColor.y;
    pass_action.colors[0].value.b = fillColor.z;
    pass_action.colors[0].value.a = fillColor.w;

    if (r3d) {
        pass_action.depth.action = SG_ACTION_CLEAR;
        pass_action.depth.value = 1.0f;
    }

    if (!started_ && rootAssetObject) {
        rootAssetObject->poll();
    }

    if (display.beginGame(pass_action)) {
        if (r3d) {
            r3d->render(display.info.size.x, display.info.size.y);
        }

        doRenderFrame();

        draw2d::begin({0, 0, display.info.size.x, display.info.size.y});

        if (!started_ && rootAssetObject) {
            drawPreloader(0.1f + 0.9f * rootAssetObject->getProgress(), display.info.size.x, display.info.size.y);
        }

        hook_on_render_frame();

        profiler.addTime("RENDER", timer.read_millis());
        profiler.addTime("FRAME", timer.read_millis());
        timer.reset();

        draw2d::end();

        display.endGame(); // beginGame()

        if (display.beginOverlayDev()) {

            onRenderOverlay();

            draw2d::begin({0, 0, g_app.drawable_size.x, g_app.drawable_size.y});
            onFrameEnd();
            profiler.draw();
            draw2d::end();

            profiler.addTime("OVERLAY", timer.read_millis());
            profiler.addTime("FRAME", timer.read_millis());
            timer.reset();

            display.endOverlayDev(); // display.beginOverlayDev()
        }

        draw2d::endFrame();
    }

    if (!started_ && asset_manager_->is_assets_ready()) {
        EK_DEBUG << "Start Game";
        onAppStart();
        onStartHook();
        started_ = true;
    }

    profiler.addTime("END", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
    timer.reset();

    profiler.update(frame_timer.delta_time());
}

void basic_application::preload_root_assets_pack() {
    rootAssetObject = asset_manager_->add_from_type("pack", "pack_meta");
    if (rootAssetObject) {
        rootAssetObject->load();
    }
}

void basic_application::on_event(const event_t& event) {
    ZoneScoped;
    timer_t timer{};
    if (event.type == event_type::app_resize) {
        display.update();
        updateRootViewport(root.get<Viewport>());
    } else if (event.type == event_type::app_close) {
        sg_shutdown();
    }

    profiler.addTime("EVENTS", timer.read_millis());
    profiler.addTime("FRAME", timer.read_millis());
}

void basic_application::doUpdateFrame(float dt) {
    hook_on_update(dt);
    scene_pre_update(root, dt);
    onUpdateFrame(dt);
    scene_post_update(root);
}

void basic_application::doRenderFrame() {
    onRenderSceneBefore();
    scene_render(root);
    onRenderSceneAfter();
}

void baseApp_drawFrame() {
    resolve<basic_application>().on_draw_frame();
}

void baseApp_onEvent(const event_t& event) {
    resolve<basic_application>().on_event(event);
}

int _initializeSubSystemsState = 0;

void initializeSubSystems() {
    const float steps = 8.0f;
    {
        EK_PROFILE_SCOPE(INIT_JOB);
        switch (_initializeSubSystemsState) {
            case 0:
                ++_initializeSubSystemsState;
                graphics::initialize();
                break;
            case 1:
                ++_initializeSubSystemsState;
                draw2d::initialize();
                break;
            case 2:
                ++_initializeSubSystemsState;
                draw2d::state.createDefaultResources();
                break;
            case 3:
                ++_initializeSubSystemsState;
                audio::initialize();
                break;
            case 4:
                ++_initializeSubSystemsState;
#ifdef EK_DEV_TOOLS
                if (Editor::inspectorEnabled) {
                    Editor::initialize();
                }
#endif
                ecs::the_world.initialize();
                registerSceneXComponents();
                break;
            case 5:
                ++_initializeSubSystemsState;
                resolve<basic_application>().initialize();
                break;
            case 6:
                ++_initializeSubSystemsState;
                resolve<basic_application>().preload();
                break;
            case 7:
                ++_initializeSubSystemsState;
                audio::start();
            default:
                g_app.on_frame_draw -= initializeSubSystems;
                g_app.on_frame_draw += baseApp_drawFrame;
                g_app.on_event += baseApp_onEvent;
                break;
        }
    }

    if (_initializeSubSystemsState > 0) {
        auto fbWidth = static_cast<int>(g_app.drawable_size.x);
        auto fbHeight = static_cast<int>(g_app.drawable_size.y);
        if (fbWidth > 0 && fbHeight > 0) {
            //EK_PROFILE_SCOPE("init frame");
            static sg_pass_action pass_action{};
            pass_action.colors[0].action = SG_ACTION_CLEAR;
            const float4 fillColor{argb32_t{g_app.window_cfg.backgroundColor}};
            pass_action.colors[0].value.r = fillColor.x;
            pass_action.colors[0].value.g = fillColor.y;
            pass_action.colors[0].value.b = fillColor.z;
            pass_action.colors[0].value.a = fillColor.w;
            sg_begin_default_pass(&pass_action, fbWidth, fbHeight);

            if (_initializeSubSystemsState > 2) {
                float2 size = g_app.drawable_size;
                draw2d::beginNewFrame();
                draw2d::begin({0, 0, size.x, size.y});
                drawPreloader(0.1f * (float) _initializeSubSystemsState / steps, size.x, size.y);
                draw2d::end();
                draw2d::endFrame();
            }

            sg_end_pass();
            sg_commit();
        }
    }
}

}
