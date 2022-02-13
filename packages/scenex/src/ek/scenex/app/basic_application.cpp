#include "basic_application.hpp"
#include "RootAppListener.hpp"

#include <ekx/app/input_state.h>

/** resource managers include **/
#include <ek/scenex/particles/ParticleDecl.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/particles/ParticleSystem.hpp>

/** systems **/
#include <ek/scenex/InteractionSystem.hpp>
#include <ekx/app/audio_manager.h>
#include <ek/scenex/2d/Viewport.hpp>
#include <ek/scenex/2d/LayoutRect.hpp>
#include <ek/scenex/systems/main_flow.hpp>
#include <ek/log.h>
#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/scenex/2d/Camera2D.hpp>

#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/Tween.hpp>
#include <ek/goodies/Shake.hpp>
#include <ek/goodies/PopupManager.hpp>
#include <ek/goodies/bubble_text.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/goodies/helpers/Trail2D.hpp>
#include <ek/scenex/base/DestroyTimer.hpp>
#include <ek/scenex/base/NodeEvents.hpp>

ek::basic_application* g_game_app = nullptr;
void init_game_app(ek::basic_application* app) {
    EK_ASSERT(!g_game_app);
    EK_ASSERT(app);
    g_game_app = app;
}

namespace ek {

void drawPreloader(float progress, float zoneWidth, float zoneHeight) {
    canvas_set_empty_image();
    auto pad = 40.0f;
    auto w = zoneWidth - pad * 2.0f;
    auto h = 16.0f;
    auto y = (zoneHeight - h) / 2.0f;

    canvas_quad_color(pad, y, w, h, COLOR_WHITE);
    canvas_quad_color(pad + 2, y + 2, w - 4, h - 4, COLOR_BLACK);
    canvas_quad_color(pad + 4, y + 4, (w - 8) * progress, h - 8, COLOR_WHITE);

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
            canvas_fill_circle({cx + ox * sw, cy - 2.0f * sh + oy * sh, R}, COLOR_WHITE, COLOR_WHITE, 16);
        }
    }
}

basic_application::basic_application() {
#ifdef EK_UITEST
    uitest::initialize(this);
#endif
}

basic_application::~basic_application() {
}

void registerSceneXComponents() {
    //// basic scene
    ECX_COMPONENT_RESERVE(Node, 512);
    ECX_COMPONENT_RESERVE(Transform2D, 512);
    ECX_COMPONENT_RESERVE(WorldTransform2D, 512);
    ECX_COMPONENT_RESERVE(Display2D, 256);
    ECX_COMPONENT(Bounds2D);
    ECX_COMPONENT(Quad2D);
    ECX_COMPONENT(Sprite2D);
    ECX_COMPONENT(NinePatch2D);
    ECX_COMPONENT(Arc2D);
    ECX_COMPONENT(Text2D);
    ECX_COMPONENT(Camera2D);
    ECX_COMPONENT(MovieClip);
    ECX_COMPONENT(MovieClipTargetIndex);

    ECX_COMPONENT(LayoutRect);
    ECX_COMPONENT(Viewport);
    ECX_COMPONENT(Button);
    ECX_COMPONENT(Interactive);
    ECX_COMPONENT(Tween);
    ECX_COMPONENT(Shaker);
    ECX_COMPONENT(BubbleText);
    ECX_COMPONENT(Trail2D);
    ECX_COMPONENT(TrailRenderer2D);
    ECX_COMPONENT(NodeEventHandler);
    ECX_COMPONENT(ParticleEmitter2D);
    ECX_COMPONENT(ParticleLayer2D);
    ECX_COMPONENT(ParticleRenderer2D);
}

void basic_application::initialize() {
    log_debug("base application: initialize");

    profiler_init();
    init_time_layers();
    log_app_display_info();
    game_display_update(&display);

    log_debug("base application: initialize scene root");
    root = createNode2D(H("root"));

    const vec2_t baseResolution = vec2(ek_app.config.width, ek_app.config.height);
    ecs::add<Viewport>(root).options.baseResolution = baseResolution;

    ecs::add<LayoutRect>(root);
    ecs::add<NodeEventHandler>(root);
    Viewport::updateAll(&display.info);
    scale_factor = ecs::get<Viewport>(root).output.scale;

    log_debug("base application: initialize InteractionSystem");
    init_interaction_system();
    g_interaction_system.root_ = root;
    log_debug("base application: initialize AudioManager");
    init_audio_manager();

    log_debug("base application: initialize Scene");
    auto camera = createNode2D(H("camera"));
    auto& defaultCamera = ecs::add<Camera2D>(camera);
    defaultCamera.root = root;
    defaultCamera.order = 1;
    defaultCamera.viewportNode = root;
    Camera2D::Main = camera;
    append(root, camera);

    LayoutRect::DesignCanvasRect = rect_size(baseResolution);
}

void basic_application::preload() {
    log_debug("base application: preloading, content scale: %d%%.", (int) (100 * scale_factor));
    asset_manager.set_scale_factor(scale_factor);
    dispatcher.onPreload();
    if (preloadOnStart) {
        preload_root_assets_pack();
    }
}

void basic_app_on_frame() {
    g_game_app->onFrame();
}

void basic_app_on_event(const ek_app_event ev) {
    g_game_app->onEvent(ev);
}

void basic_application::onFrame() {
    uint64_t timer = ek_ticks(nullptr);

    root_app_on_frame();

    dispatcher.onBeforeFrameBegin();
    game_display_update(&display);
    Viewport::updateAll(&display.info);
    scale_factor = ecs::get<Viewport>(root).output.scale;
    asset_manager.set_scale_factor(scale_factor);

    /** base app BEGIN **/

    const float dt = fminf((float)update_frame_timer(&frame_timer), 0.3f);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;
    doUpdateFrame(dt);

    double elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler_add_time(PROFILE_UPDATE, (float) (elapsed * 1000));
    profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

    canvas_new_frame();

    /// PRE-RENDER
    onPreRender();
    dispatcher.onPreRender();

    sg_pass_action pass_action{};
    pass_action.colors[0].action = started_ ? SG_ACTION_DONTCARE : SG_ACTION_CLEAR;
    const vec4_t fillColor = vec4_color(ARGB(ek_app.config.background_color));
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

    if (game_display_begin(&display, &pass_action, "game-view")) {
        profiler_render_begin(display.info.size.x * display.info.size.y);

        doRenderFrame();

        canvas_begin(display.info.size.x, display.info.size.y);

        if (!started_ && rootAssetObject) {
            drawPreloader(0.1f + 0.9f * rootAssetObject->getProgress(), display.info.size.x, display.info.size.y);
        }

        dispatcher.onRenderFrame();

        elapsed = ek_ticks_to_sec(ek_ticks(&timer));
        profiler_add_time(PROFILE_RENDER, (float) (elapsed * 1000));
        profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

        profiler_render_end();
        profiler_draw(&display.info);

        canvas_end();

        game_display_end(&display);

        if (game_display_dev_begin(&display)) {

            dispatcher.onRenderOverlay();

            canvas_begin(ek_app.viewport.width, ek_app.viewport.height);
            onFrameEnd();
            canvas_end();

            elapsed = ek_ticks_to_sec(ek_ticks(&timer));
            profiler_add_time(PROFILE_OVERLAY, (float) (elapsed * 1000));
            profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

            game_display_dev_end(&display);
        }
    }
    sg_commit();

    if (!started_ && asset_manager.is_assets_ready()) {
        log_debug("Start Game");
        onAppStart();
        dispatcher.onStart();
        started_ = true;
    }

    input_state_post_update();
    dispatcher.onPostFrame();

    elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler_add_time(PROFILE_END, (float) (elapsed * 1000));
    profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));

    profiler_update((float) frame_timer.dt);
}

void basic_application::preload_root_assets_pack() {
    rootAssetObject = new PackAsset("pack.bin");
    asset_manager.add(rootAssetObject);
    if (rootAssetObject) {
        rootAssetObject->load();
    }
}

void basic_application::onEvent(const ek_app_event event) {
    uint64_t timer = ek_ticks(nullptr);

    input_state_process_event(&event, &display.info);

    dispatcher.onEvent(event);

    root_app_on_event(event);
    if (event.type == EK_APP_EVENT_RESIZE) {
        game_display_update(&display);
    }

    double elapsed = ek_ticks_to_sec(ek_ticks(&timer));
    profiler_add_time(PROFILE_EVENTS, (float) (elapsed * 1000));
    profiler_add_time(PROFILE_FRAME, (float) (elapsed * 1000));
}

void basic_application::doUpdateFrame(float dt) {
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

void basic_application::terminate() {

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
                    ek_gfx_setup(drawCalls);
                }
#ifdef EK_DEV_TOOLS
                if (g_editor_config) {
                    init_editor();
                }
#endif
                break;
            case 1:
                ++_initializeSubSystemsState;
                canvas_setup();
                break;
            case 2:
                ++_initializeSubSystemsState;
                ecx_setup();
                registerSceneXComponents();
                break;
            case 3:
                ++_initializeSubSystemsState;
                g_game_app->initialize();
                break;
            case 4:
                ++_initializeSubSystemsState;
                g_game_app->preload();
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
            const vec4_t fillColor = vec4_color(ARGB(ek_app.config.background_color));
            pass_action.colors[0].value.r = fillColor.x;
            pass_action.colors[0].value.g = fillColor.y;
            pass_action.colors[0].value.b = fillColor.z;
            pass_action.colors[0].value.a = 1.0f;
            sg_begin_default_pass(&pass_action, (int) width, (int) height);

            if (_initializeSubSystemsState > 2) {
                canvas_new_frame();
                canvas_begin(width, height);
                drawPreloader(0.1f * (float) _initializeSubSystemsState / steps, width, height);
                canvas_end();
            }

            sg_end_pass();
            sg_commit();
        }
    }
}

void setup_resource_managers() {
    setup_res_sprite();
    setup_res_particle();
    setup_res_dynamic_atlas();
    setup_res_atlas();
    setup_res_font();
    setup_res_sg();
}

}

