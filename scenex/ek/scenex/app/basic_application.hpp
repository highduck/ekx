#pragma once

#include <ecxx/ecxx.hpp>

#include <ek/util/locator.hpp>
#include <ek/app/app.hpp>
#include <ek/util/signals.hpp>
#include "app_utils.hpp"

namespace ek {

class asset_manager_t;

class basic_application {
public:
    /**** assets ***/

    framed_timer_t frame_timer{};

    bool clear_color_enabled = false;
    float4 clear_color{0.3f, 0.3f, 0.3f, 1.0f};
    float scale_factor = 1.0f;

    signal_t<> hook_on_preload{};
    signal_t<> hook_on_draw_frame{};
    signal_t<> hook_on_render_frame{};
    signal_t<float> hook_on_update{};

    /////

    ecs::world w;
    ecs::entity root;
    ecs::entity game;
    float2 base_resolution;

    frame_stats_t stats_{};
    asset_manager_t* asset_manager_ = nullptr;

    basic_application();

    virtual ~basic_application();

    virtual void initialize();

    virtual void preload();

    void on_draw_frame();

    void on_event(const app::event_t&);

protected:
    //void on_event(const app::event_t& event);

    virtual void update_frame(float dt);

    virtual void render_frame();

    void on_frame_end();

    virtual void preload_root_assets_pack();

    virtual void start_game();

    bool started_ = false;
};

template<typename T>
inline void run_app() {
    auto& app = service_locator_instance<basic_application>::init<T>();
    app.initialize();
    app.preload();
    app::g_app.on_frame_draw += [&app] { app.on_draw_frame(); };
    app::g_app.on_event += [&app](const auto& event) { app.on_event(event); };
}

}


