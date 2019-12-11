#pragma once

#include <scenex/config/ecs.h>

#include <ek/util/locator.hpp>
#include <ek/app/app.hpp>
#include <ek/util/signals.hpp>
#include "app_utils.hpp"

namespace scenex {

class asset_manager_t;

class basic_application {
public:
    /**** assets ***/
    std::string assets_path{"assets/"};

    ek::framed_timer_t frame_timer{};

    bool clear_color_enabled = false;
    ek::float4 clear_color{0.3f, 0.3f, 0.3f, 1.0f};
    float scale_factor = 1.0f;

    ek::signal_t<> hook_on_preload{};
    ek::signal_t<> hook_on_draw_frame{};
    ek::signal_t<> hook_on_render_frame{};
    ek::signal_t<float> hook_on_update{};

    /////

    ecs::world w;
    ecs::entity root;
    ecs::entity game;
    ek::float2 base_resolution;

    frame_stats_t stats_{};
    asset_manager_t* asset_manager_ = nullptr;

    basic_application();

    virtual ~basic_application();

    virtual void initialize();

    virtual void preload();

    void on_draw_frame();

protected:
    //void on_event(const ek::app::event_t& event);

    virtual void update_frame(float dt);

    virtual void render_frame();

    void on_frame_end();

    virtual void preload_root_assets_pack();

    virtual void start_game();

    bool started_ = false;
};

template<typename T>
void run_app() {
    auto& app = ek::service_locator_instance<basic_application>::init<T>();
    app.initialize();
    app.preload();
//    ek::app::g_app.on_event += [&](const auto& e) { app.on_event(e); };
    ek::app::g_app.on_frame_draw += [&] { app.on_draw_frame(); };
}

}


