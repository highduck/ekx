#pragma once

#include <ecxx/ecxx_fwd.hpp>

#ifdef EK_DEV_TOOLS

#include <ek/editor/Editor.hpp>

#endif

#include <ek/app.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <ek/time.h>
#include <ek/util/ServiceLocator.hpp>
#include <ek/audio/audio.hpp>
#include <ek/util/Signal.hpp>
#include <utility>
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


void basic_app_on_frame();

void basic_app_on_event(ek_app_event);

class basic_application {
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

    virtual ~basic_application();

    virtual void initialize();

    virtual void preload();

    void onFrame();
    void onEvent(ek_app_event event);

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

void launcher_on_frame();

template<typename T>
inline void run_app(ek_app_config cfg) {
    ek::core::setup();
    gTextEngine.initialize();
    ek_app.config = cfg;

#ifdef EK_DEV_TOOLS
    Editor::inspectorEnabled = true;
    Editor::settings.load();
    if (Editor::settings.width > 0 && Editor::settings.height > 0) {
        ek_app.config.window_width = Editor::settings.width;
        ek_app.config.window_height = Editor::settings.height;
    }
#endif

    // audio should be initialized before "Resume" event, so the best place is "On Create" event
    audio::initialize();

    ek_app.on_ready = []{Locator::create<basic_application, T>();};
    ek_app.on_frame = launcher_on_frame;
    ek_app.on_event = root_app_on_event;
}

}


