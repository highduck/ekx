#include "base_app.hpp"

#include <ek/assets.hpp>
#include <graphics/program.hpp>
#include <ek/locator.hpp>
#include <draw2d/drawer.hpp>
#include <platform/Window.h>
#include <ek/logger.hpp>
#include <ek/timer.hpp>
#include <ek/audiomini/AudioMini.h>

namespace ek {

base_app_t::base_app_t() {
    assert_created_once<base_app_t>();
}

base_app_t::~base_app_t() = default;

void base_app_t::onKeyEvent(const KeyEvent&) {
}

void base_app_t::onMouseEvent(const MouseEvent&) {
}

void base_app_t::onTouchEvent(const TouchEvent&) {
}

void base_app_t::onAppEvent(const AppEvent& event) {
    if (event.type == AppEvent::Type::Resize) {
        EK_DEBUG << "w: " << gWindow.backBufferSize.width << " h: " << gWindow.backBufferSize.height;
    }
}

void base_app_t::onDrawFrame() {
    auto& graphics = resolve<graphics_t>();
    auto& drawer = resolve<drawer_t>();
    drawer.batcher.stats.reset();

    const float dt = std::min(frame_timer.update(), 0.3f);

    // fixed for GIF recorder
    //dt = 1.0f / 60.0f;

    update_frame(dt);
    hook_on_update(dt);

    graphics.begin();
    graphics.viewport();
    //graphics.set_scissors();

    if (clear_color_enabled) {
        graphics.clear(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
    }

    auto back_buffer_size = gWindow.backBufferSize;
    drawer.begin(0, 0, back_buffer_size.width, back_buffer_size.height);
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

    scale_factor = gWindow.scaleFactor;
}

void base_app_t::preload() {
}

}