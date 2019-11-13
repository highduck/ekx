#include "app_utils.hpp"
#include "basic_application.h"

#include <ek/assets.hpp>

#include <graphics/vertex_decl.hpp>
#include <graphics/texture.hpp>

#include <scenex/2d/atlas.hpp>
#include <scenex/2d/font.hpp>

#include <ek/locator.hpp>
#include <draw2d/drawer.hpp>
#include <ek/imaging/drawing.hpp>

using namespace ek;

namespace scenex {

void preload_builtin(const std::string& base_path) {
    asset_t<texture_t> empty_texture{"empty"};
    empty_texture.reset(new texture_t);
    image_t image_t{4, 4};
    fill_image(image_t, 0xFFFFFFFFu);
    empty_texture->upload(image_t);
}

void frame_stats_t::update() {
#ifndef NDEBUG
    auto& drawer = resolve<drawer_t>();
    auto& app = ek::resolve<basic_application>();
    fps_meter_.update(app.frame_timer.delta_time());
    fps_history_.write(fps_meter_.frame_fps());
    draw_calls_ = drawer.batcher.stats.draw_calls;
    triangles_ = drawer.batcher.stats.triangles;
#endif
}

void frame_stats_t::draw() {
#ifndef NDEBUG
    auto& drawer = resolve<drawer_t>();

    drawer.set_empty_texture();
    const int samples = fps_history_.capacity();
    float x = 0.0f;
    for (int i = 0; i < samples; ++i) {
        float fps = fps_history_.at(i);
        drawer.quad(x, 100 - fps, 2, fps, 0xFFFF00_rgb);
        x += 3.0f;
    }

    asset_t<font_t> font{"mini"};
    if (font) {
        char buf[256];
        snprintf(buf, sizeof(buf) - 1, "FPS: %0.0f\nDC: %u\nTRIS: %u", fps_meter_.fps(), draw_calls_, triangles_);
        const std::string text{buf};
        float2 pos{10.0f, 30.0f};
        font->draw(text, 16, pos + float2::one, 0x0_rgb, 16);
        font->draw(text, 16, pos, 0xFFFFFF_rgb, 16);
    }
#endif
}

}