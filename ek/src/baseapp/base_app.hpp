#pragma once

#include <ek/app/app.hpp>
#include <ek/math/vec.hpp>
#include <ek/util/signals.hpp>
#include <ek/util/timer.hpp>

namespace ek {

class base_app_t {
public:

    /**** assets ***/
    std::string assets_path{"assets/"};

    framed_timer_t frame_timer{};

    bool clear_color_enabled = false;
    float4 clear_color{0.3f, 0.3f, 0.3f, 1.0f};
    float scale_factor = 1.0f;

    signal_t<> hook_on_preload{};
    signal_t<> hook_on_draw_frame{};
    signal_t<> hook_on_render_frame{};
    signal_t<float> hook_on_update{};

    base_app_t();

    virtual ~base_app_t();

    virtual void initialize();

    virtual void preload();

    void on_event(const app::event_t& event);

    virtual void on_draw_frame();

protected:

    virtual void on_frame_end() {}

    virtual void update_frame(float) {}

    virtual void render_frame() {}
};

}


