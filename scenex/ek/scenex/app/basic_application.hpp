#pragma once

#include <ecxx/ecxx.hpp>

#ifdef EK_EDITOR

#include <ek/editor/editor.hpp>

#endif

#include <ek/util/locator.hpp>
#include <ek/app/app.hpp>
#include <ek/util/signals.hpp>
#include <utility>
#include <ek/timers.hpp>
#include "profiler.hpp"

namespace ek {

class asset_manager_t;

class asset_object_t;

class basic_application {
public:
    /**** assets ***/

    framed_timer_t frame_timer{};

    float scale_factor = 1.0f;

    signal_t<> hook_on_preload{};

    // used to render all offscreen passes
    signal_t<> onPreRender{};

    signal_t<> hook_on_draw_frame{};
    signal_t<> hook_on_render_frame{};
    signal_t<float> hook_on_update{};
    signal_t<> onStartHook{};

    /////
    ecs::entity root;
    ecs::entity game;

    static float2 AppResolution;

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
    //void on_event(const app::event_t& event);

    virtual void update_frame(float dt);

    virtual void render_frame();

    void on_frame_end();

    virtual void preload_root_assets_pack();

    virtual void start_game();

    bool started_ = false;
};

inline float2 basic_application::AppResolution{};

template<typename T>
inline void run_app(app::window_config cfg) {
    using app::g_app;
    using app::vec2;

    clock::init();
    basic_application::AppResolution = float2{cfg.size};
    g_app.window_cfg = std::move(cfg);

#ifdef EK_EDITOR
    Editor::settings.load();
    if (length(Editor::settings.windowSize) > 0.0f) {
        g_app.window_cfg.size = vec2{Editor::settings.windowSize};
        g_app.window_cfg.title = "Ekitor: " + g_app.window_cfg.title;
    }
#endif

    g_app.on_device_ready << [] {
        auto& app = service_locator_instance<basic_application>::init<T>();
#ifdef EK_EDITOR
        Editor::initialize();
#endif
        app.initialize();
        app.preload();

        g_app.on_frame_draw += [] {
            resolve<basic_application>().on_draw_frame();
        };

        g_app.on_event += [](const auto& event) {
            resolve<basic_application>().on_event(event);
        };
    };

    start_application();
}

}


