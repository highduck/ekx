#include "app_utils.hpp"
#include "basic_application.hpp"

#include <ek/util/assets.hpp>

#include <ek/graphics/vertex_decl.hpp>
#include <ek/graphics/texture.hpp>

#include <ek/scenex/text/font.hpp>

#include <ek/draw2d/drawer.hpp>
#include <ek/imaging/drawing.hpp>
#include <ek/scenex/text/text_drawer.hpp>
#include <ek/app/device.hpp>

namespace ek {

const char* program_2d_vs_ = "#ifdef GL_ES\n"
                             "precision highp float;\n"
                             "#endif\n"
                             "\n"
                             "attribute vec2 aPosition;\n"
                             "attribute vec2 aTexCoord;\n"
                             "attribute vec4 aColorMult;\n"
                             "attribute vec4 aColorOffset;\n"
                             "\n"
                             "uniform mat4 uModelViewProjection;\n"
                             "\n"
                             "varying vec2 vTexCoord;\n"
                             "varying vec4 vColorMult;\n"
                             "varying vec4 vColorOffset;\n"
                             "\n"
                             "void main() {\n"
                             "    vTexCoord = aTexCoord;\n"
                             "    vColorMult = aColorMult;\n"
                             "    vColorOffset = aColorOffset;\n"
                             "    gl_Position = uModelViewProjection * vec4(aPosition, 0.0, 1.0);\n"
                             "}";

const char* program_2d_fs_ = "#ifdef GL_ES\n"
                             "precision mediump float;\n"
                             "#endif\n"
                             "\n"
                             "varying vec2 vTexCoord;\n"
                             "varying vec4 vColorMult;\n"
                             "varying vec4 vColorOffset;\n"
                             "\n"
                             "uniform sampler2D uImage0;\n"
                             "\n"
                             "void main() {\n"
                             "    vec4 pixelColor = texture2D(uImage0, vTexCoord);\n"
                             "    pixelColor *= vColorMult;\n"
                             "    gl_FragColor = pixelColor + vColorOffset * pixelColor.wwww;\n"
                             "}";

void create_builtin() {
    using namespace graphics;
    asset_t<texture_t> empty_texture{"empty"};
    empty_texture.reset(new texture_t);
    image_t image_t{4, 4};
    fill_image(image_t, 0xFFFFFFFFu);
    empty_texture->upload(image_t);

    auto pr = new program_t(program_2d_vs_, program_2d_fs_);
    pr->vertex = &vertex_2d::decl;
    asset_t<program_t>{"2d"}
            .reset(pr);
}

void frame_stats_t::update() {
#ifndef NDEBUG
    auto& app = resolve<basic_application>();
    fps_meter_.update(app.frame_timer.delta_time());
    millis_history_.write(app.frame_timer.delta_time());
    draw_calls_ = draw2d::get_stat_draw_calls();
    triangles_ = draw2d::get_stat_triangles();
#endif
}

void frame_stats_t::draw() {
#ifndef NDEBUG
    const auto scale = static_cast<float>(app::g_app.content_scale);
    auto insets = get_screen_insets();
    draw2d::state.save_matrix()
            .translate(insets[0], insets[1])
            .scale(scale, scale);
    draw2d::state.set_empty_texture();

    static char buf[256];
    static TextDrawer textDrawer;
    textDrawer.format.font = asset_t<Font>("Cousine-Regular");
    textDrawer.format.size = 16.0f;
    textDrawer.format.leading = 1.0f;
    textDrawer.format.layers[0].color = 0xFFFFFF_rgb;
    textDrawer.format.layers[1].blurRadius = 0;
    textDrawer.format.layers[1].offset = {1, 1};
    textDrawer.format.layers[1].blurIterations = 2;
    textDrawer.format.layers[1].strength = 0;
    textDrawer.format.layers[1].color = 0x0_rgb;
    textDrawer.format.layersCount = 2;

    const int samples = millis_history_.capacity();
    float x = 0.0f;
    float prev = 1000.0f * millis_history_.at(0);
    float H = 80;
    draw2d::quad(0, 0, 128, H, 0x77000000_argb);
    for (int i = 0; i < samples; ++i) {
        float val = 1000.0f * millis_history_.at(i);
        auto thMin = 1000.0f / 60.0f;
        auto thMax = 1000.0f / 30.0f;
        draw2d::quad(x, H - val, 1, val - prev + 1,
                     lerp(0x00FF00_rgb, 0xFF0000_rgb, math::clamp((val - thMin) / (thMax - thMin), 0.0f, 1.0f)));
        prev = val;
        x += 1;
    }
    snprintf(buf, sizeof(buf) - 1, "%0.2f ms", prev);
    textDrawer.rect.position = {100, H - prev};
    textDrawer.draw(buf);

    snprintf(buf, sizeof(buf) - 1, u8"FPS: %0.0f\nDC: %u\nTRIS: %u",
             fps_meter_.fps(),
             draw_calls_,
             triangles_);
    textDrawer.rect.position = {5.0f, 20.0f};
    textDrawer.draw(buf);

    draw2d::state.restore_matrix();
#endif
}

}