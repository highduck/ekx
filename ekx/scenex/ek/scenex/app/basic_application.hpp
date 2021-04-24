#pragma once

#include <ecxx/ecxx_fwd.hpp>

#ifdef EK_DEV_TOOLS

#include <ek/editor/editor.hpp>

#endif

#include <ek/util/locator.hpp>
#include <ek/app/app.hpp>
#include <ek/util/logger.hpp>
#include <ek/util/signals.hpp>
#include <utility>
#include <ek/timers.hpp>
#include "profiler.hpp"
#include <ek/imaging/ImageSubSystem.hpp>
#include "GameDisplay.hpp"

namespace ek {

class asset_manager_t;

class asset_object_t;

class basic_application {
public:
    static float2 AppResolution;
    inline static GameDisplayInfo currentDisplayInfo{};

    GameDisplay display{};
    /**** assets ***/

    framed_timer_t frame_timer{};

    float scale_factor = 1.0f;

    signal_t<> hook_on_preload{};

    // used to render all offscreen passes
    signal_t<> onPreRender{};

    signal_t<> onRenderOverlay{};
    signal_t<> hook_on_render_frame{};
    signal_t<float> hook_on_update{};
    signal_t<> onStartHook{};

    /////
    ecs::EntityApi root;

    Profiler profiler{};
    asset_manager_t* asset_manager_ = nullptr;

    basic_application();

    virtual ~basic_application();

    virtual void initialize();

    virtual void preload();

    void on_draw_frame();

    void on_event(const app::event_t&);

public:
    bool preloadOnStart = true;
    asset_object_t* rootAssetObject = nullptr;

protected:
    void doUpdateFrame(float dt);

    void doRenderFrame();

    virtual void preload_root_assets_pack();

    bool started_ = false;


    // after root pack resources are loaded
    virtual void onAppStart() {}

    virtual void onUpdateFrame(float dt) { (void) dt; }

    virtual void onRenderSceneBefore() {}

    virtual void onRenderSceneAfter() {}

    virtual void onFrameEnd() {}
};

inline float2 basic_application::AppResolution{};

void initializeSubSystems();

template<typename T>
inline void run_app(app::window_config cfg) {
    using app::g_app;

    memory::initialize();
    app::initialize();
    clock::initialize();
    imaging::initialize();
    basic_application::AppResolution = float2{cfg.size};
    g_app.window_cfg = std::move(cfg);

#ifdef EK_DEV_TOOLS
    Editor::inspectorEnabled = true;
    Editor::settings.load();
    if (length(Editor::settings.windowSize) > 0.0f) {
        g_app.window_cfg.size = Editor::settings.windowSize;
    }
#endif

    g_app.on_device_ready << [] {
        service_locator_instance<basic_application>::init<T>();
        g_app.on_frame_draw += initializeSubSystems;
    };

    EK_TRACE << "app: call start_application";
    start_application();
}

}


