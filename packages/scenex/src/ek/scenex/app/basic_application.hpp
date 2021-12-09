#pragma once

#include <ecxx/ecxx_fwd.hpp>

#ifdef EK_DEV_TOOLS

#include <ek/editor/Editor.hpp>

#endif

#include <ek/util/ServiceLocator.hpp>
#include <ek/app/app.hpp>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/audio/audio.hpp>
#include <ek/util/Signal.hpp>
#include <utility>
#include <ek/time.h>
#include <ek/time/Timers.hpp>
#include "profiler.hpp"
#include "GameDisplay.hpp"
#include "GameAppDispatcher.hpp"
#include "../text/TextEngine.hpp"
#include "RootAppListener.hpp"

#include "ek/core.hpp"

namespace ek {

class AssetManager;

class Asset;

class FrameTimer final {
public:
    double deltaTime = 0.0;
    uint64_t frameIndex = 0;

    double update();

private:
    uint64_t timer_ = ek_ticks(nullptr);
};

class basic_application : public RootAppListener {
public:
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

class Initializer : public RootAppListener {
public:
    void (* creator)() = nullptr;

    int _initializeSubSystemsState = 0;

    ~Initializer() override = default;

    void onReady() override;

    void onFrame() override;
};

template<typename T>
inline void run_app(app::AppConfig cfg) {
    using app::g_app;

    ek::core::setup();
    gTextEngine.initialize();
    g_app.config = cfg;

#ifdef EK_DEV_TOOLS
    Editor::inspectorEnabled = true;
    Editor::settings.load();
    if (Editor::settings.width > 0.0f && Editor::settings.height > 0.0f) {
        g_app.config.windowWidth = Editor::settings.width;
        g_app.config.windowHeight = Editor::settings.height;
    }
#endif

    // audio should be initialized before "Resume" event, so the best place is "On Create" event
    audio::initialize();

    static Initializer initializer;
    initializer.creator = []{Locator::create<basic_application, T>();};
    g_app.listener = &initializer;
}

}


