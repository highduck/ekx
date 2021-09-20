#include "basic_application.hpp"

#include "input_controller.hpp"

#include <ek/scenex/InteractionSystem.hpp>
#include <ek/scenex/AudioManager.hpp>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/debug.hpp>
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
#include <Tracy.hpp>

#include "impl/GameDisplay_impl.hpp"

#ifdef EK_UITEST
#include "impl/uitest_impl.hpp"
#endif

namespace ek {

using namespace ek::app;

void logDisplayInfo() {
#ifndef NDEBUG
    EK_INFO_F("Display: %f x %f", g_app.drawableWidth, g_app.drawableHeight);
    const float* insets = app::getScreenInsets();
    if (insets) {
        EK_INFO_F("Insets: %f, %f, %f, %f", insets[0], insets[1], insets[2], insets[3]);
    }
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
    asset_manager_ = new AssetManager{};
}

basic_application::~basic_application() {

    ecs::the_world.shutdown();

    draw2d::shutdown();

    delete asset_manager_;

    audio::shutdown();
    graphics::shutdown();
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
    ZoneScopedN("initialize");

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

    const float2 baseResolution{app::g_app.config.width, app::g_app.config.height};
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

    LayoutRect::DesignCanvasRect = {float2::zero, baseResolution};
}

void basic_application::preload() {
    EK_DEBUG_F("base application: preloading, content scale: %d perc.", (int) (100 * scale_factor));
    asset_manager_->set_scale_factor(scale_factor);

    dispatcher.onPreload();

    asset_manager_->add_resolver(new DefaultAssetsResolver());
    if (preloadOnStart) {
        preload_root_assets_pack();
    }
}

void basic_application::onFrame() {
    ZoneScoped;
    Stopwatch timer{};

    RootAppListener::onFrame();

    dispatcher.onBeforeFrameBegin();
    display.update();
    scale_factor = root.get<Viewport>().output.scale;
    asset_manager_->set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const float dt = fmin(frameTimer.update(), 0.3f);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;
    doUpdateFrame(dt);
    profiler.addTime("UPDATE", timer.readMillis());
    profiler.addTime("FRAME", timer.readMillis());
    timer.reset();

    draw2d::beginNewFrame();

    /// PRE-RENDER
    onPreRender();
    dispatcher.onPreRender();

    static sg_pass_action pass_action{};
    pass_action.colors[0].action = started_ ? SG_ACTION_DONTCARE : SG_ACTION_CLEAR;
    const float4 fillColor = static_cast<float4>(argb32_t{g_app.config.backgroundColor});
    pass_action.colors[0].value.r = fillColor.x;
    pass_action.colors[0].value.g = fillColor.y;
    pass_action.colors[0].value.b = fillColor.z;
    pass_action.colors[0].value.a = fillColor.w;
    if(app::g_app.config.needDepth) {
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

        profiler.addTime("RENDER", timer.readMillis());
        profiler.addTime("FRAME", timer.readMillis());
        timer.reset();

        profiler.endRender();
        draw(profiler, display.info);

        draw2d::end();

        display.endGame(); // beginGame()

        if (display.beginOverlayDev()) {

            dispatcher.onRenderOverlay();

            draw2d::begin({0, 0, g_app.drawableWidth, g_app.drawableHeight});
            onFrameEnd();
            draw2d::end();

            profiler.addTime("OVERLAY", timer.readMillis());
            profiler.addTime("FRAME", timer.readMillis());
            timer.reset();

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

    profiler.addTime("END", timer.readMillis());
    profiler.addTime("FRAME", timer.readMillis());
    timer.reset();

    profiler.update(frameTimer.deltaTime);
}

void basic_application::preload_root_assets_pack() {
    rootAssetObject = asset_manager_->add_file("pack.bin", "pack");
    if (rootAssetObject) {
        rootAssetObject->load();
    }
}

void basic_application::onEvent(const Event& event) {
    ZoneScoped;
    Stopwatch timer{};

    RootAppListener::onEvent(event);
    if (event.type == EventType::Resize) {
        display.update();
    }

    Locator::ref<input_controller>().onEvent(event);

    dispatcher.onEvent(event);

    profiler.addTime("EVENTS", timer.readMillis());
    profiler.addTime("FRAME", timer.readMillis());
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

void Initializer::onReady() {
    if (creator != nullptr) {
        creator();
    }
}

void Initializer::onFrame() {
    RootAppListener::onFrame();

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
                    graphics::initialize(drawCalls);
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
                g_app.listener = Locator::get<basic_application>();
                break;
        }
    }

    if (_initializeSubSystemsState > 0) {
        const auto width = g_app.drawableWidth;
        const auto height = g_app.drawableHeight;
        if (width > 0 && height > 0) {
            //EK_PROFILE_SCOPE("init frame");
            static sg_pass_action pass_action{};
            pass_action.colors[0].action = SG_ACTION_CLEAR;
            const float4 fillColor = static_cast<float4>(argb32_t{g_app.config.backgroundColor});
            pass_action.colors[0].value.r = fillColor.x;
            pass_action.colors[0].value.g = fillColor.y;
            pass_action.colors[0].value.b = fillColor.z;
            pass_action.colors[0].value.a = fillColor.w;
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
