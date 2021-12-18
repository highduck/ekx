#include "basic_application.hpp"

#include "input_controller.hpp"

#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/log.h>
#include <ek/scenex/asset2/Asset.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/scenex/2d/Camera2D.hpp>

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

#include "impl/GameDisplay_impl.hpp"

#ifdef EK_UITEST
#include "impl/uitest_impl.hpp"
#endif

namespace ek {

double FrameTimer::update() {
    ++frameIndex;
    return deltaTime = ek_ticks_to_sec(ek_ticks(&timer_));
}

void logDisplayInfo() {
#ifndef NDEBUG
    EK_INFO("Display: %d x %d", (int) ek_app.viewport.width, (int) ek_app.viewport.height);
    EK_INFO("Insets: %d, %d, %d, %d",
            (int) ek_app.viewport.insets[0],
            (int) ek_app.viewport.insets[1],
            (int) ek_app.viewport.insets[2],
            (int) ek_app.viewport.insets[3]);
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

    {
        float sz = zoneWidth < zoneHeight ? zoneWidth : zoneHeight;
        float cx = zoneWidth / 2.0f;
        float cy = zoneHeight / 2.0f;
        float sh = sz / 16.0f;
        float sw = sh * 3;
        const auto time = (float) ek_time_now();
        for (int i = 0; i < 7; ++i) {
            float r = ((float) i / 7) * 1.5f + time;
            float speed = (0.5f + 0.5f * sinf(r * 2 + 1));
            r = r + 0.5f * speed;
            float ox = sinf(r * 3.14f);
            float oy = sinf(r * 3.14f * 2 + 3.14f);
            float R = (sh / 10.0f) *
                      (1.8f - 0.33f * speed - 0.33f * ((cosf(r * 3.14f) + 2.0f * cosf(r * 3.14f * 2 + 3.14f))));
            draw2d::fill_circle({cx + ox * sw, cy - 2.0f * sh + oy * sh, R}, 0xFFFFFFFF_argb, 0xFFFFFFFF_argb, 16);
        }
    }
}

basic_application::basic_application() {
#ifdef EK_UITEST
    uitest::initialize(this);
#endif
    asset_manager_ = new AssetManager{};
}

basic_application::~basic_application() {

    ecs::the_world.shutdown();

    draw2d::shutdown();

    delete asset_manager_;

    audio::shutdown();
    ek_gfx_shutdown();
}

void registerSceneXComponents() {
    //// basic scene
    using ecs::the_world;
    ECX_COMPONENT_RESERVE(Node, 512);
    ECX_COMPONENT_RESERVE(NodeName, 512);
    ECX_COMPONENT_RESERVE(Transform2D, 512);
    ECX_COMPONENT_RESERVE(WorldTransform2D, 512);
    ECX_COMPONENT_RESERVE(Display2D, 256);
    ECX_COMPONENT(Camera2D);
    ECX_COMPONENT(Bounds2D);
    ECX_COMPONENT(MovieClip);
    ECX_COMPONENT(MovieClipTargetIndex);

    ECX_COMPONENT(LayoutRect);
    ECX_COMPONENT(Viewport);
    ECX_COMPONENT(Button);
    ECX_COMPONENT(Interactive);
    ECX_COMPONENT(Tween);
    ECX_COMPONENT(Shake);
    ECX_COMPONENT(Shaker);
    ECX_COMPONENT(ScriptHolder);
    ECX_COMPONENT(Updater);
    ECX_COMPONENT(BubbleText);
    ECX_COMPONENT(PopupManager);
    ECX_COMPONENT(close_timeout);
    ECX_COMPONENT(GameScreen);
    ECX_COMPONENT(Trail2D);
    ECX_COMPONENT(DestroyTimer);
    ECX_COMPONENT(NodeEventHandler);
    ECX_COMPONENT(ParticleEmitter2D);
    ECX_COMPONENT(ParticleLayer2D);
    ECX_COMPONENT(UglyFilter2D);
}

void basic_application::initialize() {
    EK_DEBUG("base application: initialize");
    // init default empty sprite data
    {
        auto* spr = new Sprite();
        spr->texture.setID("empty");
        Res<Sprite>{"empty"}.reset(spr);
    }

    logDisplayInfo();
    display.update();

    EK_DEBUG("base application: initialize scene root");
    root = createNode2D("root");

    const Vec2f baseResolution{ek_app.config.width, ek_app.config.height};
    root.assign<Viewport>(baseResolution.x, baseResolution.y);

    root.assign<LayoutRect>();
    root.assign<NodeEventHandler>();
    Viewport::updateAll(display.info);
    scale_factor = root.get<Viewport>().output.scale;

    EK_DEBUG("base application: initialize InteractionSystem");
    auto& im = Locator::create<InteractionSystem>(root);
    EK_DEBUG("base application: initialize input_controller");
    Locator::create<input_controller>(im, display);
    EK_DEBUG("base application: initialize AudioManager");
    Locator::create<AudioManager>();

    EK_DEBUG("base application: initialize Scene");
    auto camera = createNode2D("camera");
    auto& defaultCamera = camera.assign<Camera2D>(root);
    defaultCamera.order = 1;
    defaultCamera.viewportNode = ecs::EntityRef{root};
    Camera2D::Main = camera;
    append(root, camera);

    LayoutRect::DesignCanvasRect = {Vec2f::zero, baseResolution};
}

void basic_application::preload() {
    EK_DEBUG("base application: preloading, content scale: %d perc.", (int) (100 * scale_factor));
    asset_manager_->set_scale_factor(scale_factor);

    dispatcher.onPreload();

    asset_manager_->add_resolver(new DefaultAssetsResolver());
    if (preloadOnStart) {
        preload_root_assets_pack();
    }
}

void basic_app_on_frame() {
    Locator::get<basic_application>()->onFrame();
}

void basic_app_on_event(const ek_app_event ev) {
    Locator::get<basic_application>()->onEvent(ev);
}

void basic_application::onFrame() {
    uint64_t timer = ek_ticks(nullptr);

    root_app_on_frame();

    dispatcher.onBeforeFrameBegin();
    display.update();
    scale_factor = root.get<Viewport>().output.scale;
    asset_manager_->set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const float dt = (float) fmin(frameTimer.update(), 0.3);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;
    doUpdateFrame(dt);

    double elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler.addTime("UPDATE", (float) (elapsed * 1000));
    profiler.addTime("FRAME", (float) (elapsed * 1000));

    draw2d::beginNewFrame();

    /// PRE-RENDER
    onPreRender();
    dispatcher.onPreRender();

    sg_pass_action pass_action{};
    pass_action.colors[0].action = started_ ? SG_ACTION_DONTCARE : SG_ACTION_CLEAR;
    const Vec4f fillColor = static_cast<Vec4f>(argb32_t{ek_app.config.background_color});
    pass_action.colors[0].value.r = fillColor.x;
    pass_action.colors[0].value.g = fillColor.y;
    pass_action.colors[0].value.b = fillColor.z;
    pass_action.colors[0].value.a = 1.0f;
    if (ek_app.config.need_depth) {
        pass_action.depth.action = SG_ACTION_CLEAR;
        pass_action.depth.value = 1.0f;
    }

    if (!started_ && rootAssetObject) {
        rootAssetObject->poll();
    }

    if (display.beginGame(pass_action)) {
        profiler.beginRender(display.info.size.x * display.info.size.y);

        doRenderFrame();

        draw2d::begin({0, 0, display.info.size.x, display.info.size.y});

        if (!started_ && rootAssetObject) {
            drawPreloader(0.1f + 0.9f * rootAssetObject->getProgress(), display.info.size.x, display.info.size.y);
        }

        dispatcher.onRenderFrame();

        elapsed = ek_ticks_to_sec(ek_ticks(&timer));
        profiler.addTime("RENDER", (float) (elapsed * 1000));
        profiler.addTime("FRAME", (float) (elapsed * 1000));

        profiler.endRender();
        draw(profiler, display.info);

        draw2d::end();

        display.endGame(); // beginGame()

        if (display.beginOverlayDev()) {

            dispatcher.onRenderOverlay();

            draw2d::begin({0, 0, ek_app.viewport.width, ek_app.viewport.height});
            onFrameEnd();
            draw2d::end();

            elapsed = ek_ticks_to_sec(ek_ticks(&timer));
            profiler.addTime("OVERLAY", (float) (elapsed * 1000));
            profiler.addTime("FRAME", (float) (elapsed * 1000));

            display.endOverlayDev(); // display.beginOverlayDev()
        }

        draw2d::endFrame();
    }
    sg_commit();

    if (!started_ && asset_manager_->is_assets_ready()) {
        EK_DEBUG("Start Game");
        onAppStart();
        dispatcher.onStart();
        started_ = true;
    }

    Locator::ref<input_controller>().onPostFrame();
    dispatcher.onPostFrame();

    elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler.addTime("END", (float) (elapsed * 1000));
    profiler.addTime("FRAME", (float) (elapsed * 1000));

    profiler.update((float) frameTimer.deltaTime);
}

void basic_application::preload_root_assets_pack() {
    rootAssetObject = asset_manager_->add_file("pack.bin", "pack");
    if (rootAssetObject) {
        rootAssetObject->load();
    }
}

void basic_application::onEvent(const ek_app_event event) {
    uint64_t timer = ek_ticks(nullptr);

    root_app_on_event(event);
    if (event.type == EK_APP_EVENT_RESIZE) {
        display.update();
    }

    Locator::ref<input_controller>().onEvent(event);

    dispatcher.onEvent(event);

    double elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler.addTime("EVENTS", (float) (elapsed * 1000));
    profiler.addTime("FRAME", (float) (elapsed * 1000));
}

void basic_application::doUpdateFrame(float dt) {
    Viewport::updateAll(display.info);
    dispatcher.onUpdate();
    scene_pre_update(root, dt);
    onUpdateFrame(dt);
    scene_post_update(root);
}

void basic_application::doRenderFrame() {
    onRenderSceneBefore();
    scene_render(root);
    onRenderSceneAfter();
}

void launcher_on_frame() {
    root_app_on_frame();

    static int _initializeSubSystemsState = 0;

    const float steps = 5.0f;
    {
        //EK_PROFILE_SCOPE(INIT_JOB);
        switch (_initializeSubSystemsState) {
            case 0:
                ++_initializeSubSystemsState;
                {
#ifdef EK_DEV_TOOLS
                    const int drawCalls = 1024;
#else
                    const int drawCalls = 128;
#endif
                    ek_gfx_init(drawCalls);
                }
#ifdef EK_DEV_TOOLS
                if (Editor::inspectorEnabled) {
                    Editor::initialize();
                }
#endif
                break;
            case 1:
                ++_initializeSubSystemsState;
                draw2d::initialize();
                break;
            case 2:
                ++_initializeSubSystemsState;
                ecs::the_world.initialize();
                registerSceneXComponents();
                break;
            case 3:
                ++_initializeSubSystemsState;
                Locator::ref<basic_application>().initialize();
                break;
            case 4:
                ++_initializeSubSystemsState;
                Locator::ref<basic_application>().preload();
                break;
            default:
                ek_app.on_frame = basic_app_on_frame;
                ek_app.on_event = basic_app_on_event;
                break;
        }
    }

    if (_initializeSubSystemsState > 0) {
        const auto width = ek_app.viewport.width;
        const auto height = ek_app.viewport.height;
        if (width > 0 && height > 0) {
            //EK_PROFILE_SCOPE("init frame");
            sg_pass_action pass_action{};
            pass_action.colors[0].action = SG_ACTION_CLEAR;
            const Vec4f fillColor = static_cast<Vec4f>(argb32_t{ek_app.config.background_color});
            pass_action.colors[0].value.r = fillColor.x;
            pass_action.colors[0].value.g = fillColor.y;
            pass_action.colors[0].value.b = fillColor.z;
            pass_action.colors[0].value.a = 1.0f;
            sg_begin_default_pass(&pass_action, (int) width, (int) height);

            if (_initializeSubSystemsState > 2) {
                draw2d::beginNewFrame();
                draw2d::begin({0, 0, width, height});
                drawPreloader(0.1f * (float) _initializeSubSystemsState / steps, width, height);
                draw2d::end();
                draw2d::endFrame();
            }

            sg_end_pass();
            sg_commit();
        }
    }
}

}
