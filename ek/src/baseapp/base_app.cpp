#include "base_app.hpp"

#include <ek/util/assets.hpp>
#include <graphics/program.hpp>
#include <ek/util/locator.hpp>
#include <draw2d/drawer.hpp>
#include <ek/util/logger.hpp>
#include <ek/audiomini.hpp>
#include <ek/app/app.hpp>

namespace ek {

base_app_t::base_app_t() {
    assert_created_once<base_app_t>();
}

base_app_t::~base_app_t() = default;

void base_app_t::on_event(const event_t& event) {
    if (event.type == event_type::app_resize) {
        EK_TRACE("size: %lf x %lf", g_app.drawable_size.x, g_app.drawable_size.y);
    }
}

void base_app_t::on_draw_frame() {
    auto& graphics = resolve<graphics_t>();
    auto& drawer = resolve<drawer_t>();
    drawer.batcher.stats.reset();

    const double dt = std::min(frame_timer.update(), 0.3);
    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;

    hook_on_update(dt);
    update_frame(dt);

    graphics.begin();
    graphics.viewport();
    //graphics.set_scissors();

    if (clear_color_enabled) {
        graphics.clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    }

    drawer.begin(0, 0,
                 static_cast<int>(g_app.drawable_size.x),
                 static_cast<int>(g_app.drawable_size.y));
    drawer.set_blend_mode(blend_mode::premultiplied);

    render_frame();

    hook_on_render_frame();

    on_frame_end();

    drawer.end();

    hook_on_draw_frame();
}

void base_app_t::initialize() {
    service_locator_instance<timer_t>::init();
    service_locator_instance<graphics_t>::init();
    service_locator_instance<drawer_t>::init();
    service_locator_instance<AudioMini>::init();

    scale_factor = static_cast<float>(g_app.content_scale);
}

void base_app_t::preload() {
}

}