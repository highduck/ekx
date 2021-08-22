#pragma once

#include <ecxx/ecxx_fwd.hpp>

#ifdef EK_DEV_TOOLS

#include <ek/editor/Editor.hpp>

#endif

#include <ek/util/ServiceLocator.hpp>
#include <ek/app/app.hpp>
#include <ek/debug.hpp>
#include <ek/audio/audio.hpp>
#include <ek/debug/LogSystem.hpp>
#include <ek/util/Signal.hpp>
#include <utility>
#include <ek/time/FrameTimer.hpp>
#include <ek/time/Clock.hpp>
#include <ek/time/Timers.hpp>
#include "profiler.hpp"
#include <ek/imaging/ImageSubSystem.hpp>
#include "GameDisplay.hpp"
#include "GameAppDispatcher.hpp"
#include "../base/SxMemory.hpp"
#include "../text/TextEngine.hpp"

#include "ek/core.hpp"

namespace ek {

class AssetManager;

class Asset;

class basic_application {
public:
    static float2 AppResolution;
    inline static GameDisplayInfo currentDisplayInfo{};

    GameDisplay display{};
    /**** assets ***/

    FrameTimer frameTimer{};

    float scale_factor = 1.0f;

    GameAppDispatcher dispatcher{};

    /////
    ecs::EntityApi root;

    Profiler profiler{};
    AssetManager* asset_manager_ = nullptr;

    basic_application();

    virtual ~basic_application();

    virtual void initialize();

    virtual void preload();

    void on_draw_frame();

    void on_event(const app::event_t&);

public:
    bool preloadOnStart = true;
    Asset* rootAssetObject = nullptr;

protected:
    void doUpdateFrame(float dt);

    void doRenderFrame();

    virtual void preload_root_assets_pack();

    bool started_ = false;


    // after root pack resources are loaded
    virtual void onAppStart() {}

    virtual void onUpdateFrame(float dt) { (void) dt; }

    virtual void onPreRender() {}

    virtual void onRenderSceneBefore() {}

    virtual void onRenderSceneAfter() {}

    virtual void onFrameEnd() {}
};

EK_DECLARE_TYPE(basic_application);

inline float2 basic_application::AppResolution{};

void initializeSubSystems();

template<typename T>
inline void run_app(app::window_config cfg) {
    using app::g_app;

    ek::core::setup();

    SxMemory.initialize();
    gTextEngine.initialize();
    Locator::setup();
    app::initialize();
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
        // audio should be initialized before "Resume" event, so the best place is "On Create" event
        audio::initialize();

        Locator::create<basic_application, T>();
        g_app.on_frame_draw += initializeSubSystems;
    };

    EK_TRACE << "app: call start_application";
    start_application();
}

}


