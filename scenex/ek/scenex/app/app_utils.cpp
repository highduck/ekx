#include "app_utils.hpp"
#include "basic_application.hpp"

#include <ek/util/assets.hpp>

#include <ek/graphics/vertex_decl.hpp>
#include <ek/graphics/texture.hpp>

#include <ek/scenex/text/font.hpp>

#include <ek/draw2d/drawer.hpp>
#include <ek/imaging/drawing.hpp>
#include <ek/scenex/text/text_drawer.hpp>

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
    fps_history_.write(fps_meter_.frame_fps());
    draw_calls_ = draw2d::get_stat_draw_calls();
    triangles_ = draw2d::get_stat_triangles();
#endif
}

void frame_stats_t::draw() {
#ifndef NDEBUG
    draw2d::state.set_empty_texture();
    const int samples = fps_history_.capacity();
    float x = 0.0f;
    for (int i = 0; i < samples; ++i) {
        float fps = fps_history_.at(i);
        draw2d::quad(x, 100 - fps, 2, fps, 0xFFFF00_rgb);
        x += 3.0f;
    }

    asset_t<Font> font{"mini"};
    if (font) {
        char buf[256];
        snprintf(buf, sizeof(buf) - 1, u8"FPS: %0.0f\nDC: %u\nTRIS: %u",
                 fps_meter_.fps(),
                 draw_calls_,
                 triangles_);

        TextDrawer textDrawer;
        textDrawer.font = asset_t<Font>("mini");
        textDrawer.fontSize = 16.0f;// * app::g_app.content_scale;
        textDrawer.position = {10.0f, 30.0f};
        textDrawer.textColor = 0x0_rgb;
        const std::string text{buf};
        textDrawer.draw(text);
        textDrawer.position -= {1.0f, 1.0f};
        textDrawer.textColor = 0xFFFFFF_rgb;
        textDrawer.draw(text);
    }
#endif
}

}