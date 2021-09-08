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

class basic_application : public app::AppListener {
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

    ~basic_application() override;

    virtual void initialize();

    virtual void preload();

    void onFrame() override;

    void onPostFrame() override;

    void onEvent(const app::Event&) override;

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

class Initializer : public app::AppListener {
public:
    void (* creator)() = nullptr;

    int _initializeSubSystemsState = 0;

    ~Initializer() override = default;

    void onPreStart() override;

    void onDeviceReady() override;

    void onFrame() override;
};

template<typename T>
inline void run_app(app::WindowConfig cfg) {
    using app::g_app;

    ek::core::setup();

    SxMemory.initialize();
    gTextEngine.initialize();
    Locator::setup();
    app::initialize();
    basic_application::AppResolution = float2{cfg.width, cfg.height};
    g_app.config = cfg;

#ifdef EK_DEV_TOOLS
    Editor::inspectorEnabled = true;
    Editor::settings.load();
    if (Editor::settings.width > 0.0f && Editor::settings.height > 0.0f) {
        g_app.config.width = Editor::settings.width;
        g_app.config.height = Editor::settings.height;
    }
#endif

    static Initializer initializer;
    initializer.creator = []{Locator::create<basic_application, T>();};
    g_app.listener = &initializer;

    EK_TRACE << "app: call start_application";
    start_application();
}

}


