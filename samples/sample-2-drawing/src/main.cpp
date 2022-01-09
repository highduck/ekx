#include <ek/app.h>
#include <ek/core.hpp>
//#include <ek/audio/audio.hpp>
#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/time.h>
#include <ek/math/Color32.hpp>

using namespace ek;

void drawPreloader(float progress, float zoneWidth, float zoneHeight) {
    canvas_set_empty_image();
    auto pad = 40.0f;
    auto w = zoneWidth - pad * 2.0f;
    auto h = 16.0f;
    auto y = (zoneHeight - h) * 0.5f + h;

    canvas_quad_color(pad, y, w, h, 0xFFFFFFFF_argb);
    canvas_quad_color(pad + 2, y + 2, w - 4, h - 4, 0xFF000000_argb);
    canvas_quad_color(pad + 4, y + 4, (w - 8) * progress, h - 8, 0xFFFFFFFF_argb);

    float sz = zoneWidth < zoneHeight ? zoneWidth : zoneHeight;
    float cx = zoneWidth / 2.0f;
    float cy = zoneHeight / 2.0f;
    float sh = sz / 16.0f;
    float sw = sh * 3;
    const auto time = (float)ek_time_now();
    for (int i = 0; i < 7; ++i) {
        float r = ((float) i / 7) * 1.5f + time;
        float speed = (0.5f + 0.5f * sinf(r * 2 + 1));
        r = r + 0.5f * speed;
        float ox = sinf(r * 3.14f);
        float oy = sinf(r * 3.14f * 2 + 3.14f);
        float R = (sh / 10.0f) *
                  (1.8f - 0.33f * speed - 0.33f * ((cosf(r * 3.14f) + 2.0f * cosf(r * 3.14f * 2 + 3.14f))));
        vec3_t circ = vec3(cx + ox * sw, cy - 2.0f * sh + oy * sh, R);
        canvas_fill_circle(circ, 0xFFFFFFFF_argb, 0xFFFFFFFF_argb, 16);
    }
}

void on_ready() {
    ek_gfx_setup(128);
    canvas_setup();
}

void on_frame() {
    const auto width = ek_app.viewport.width;
    const auto height = ek_app.viewport.height;
    if (width > 0 && height > 0) {
        static sg_pass_action pass_action{};
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        const vec4_t fillColor = vec4_rgba(argb32_t{ek_app.config.background_color});
        pass_action.colors[0].value.r = fillColor.x;
        pass_action.colors[0].value.g = fillColor.y;
        pass_action.colors[0].value.b = fillColor.z;
        pass_action.colors[0].value.a = 1.0f;
        sg_begin_default_pass(&pass_action, (int) width, (int) height);

        canvas_new_frame();
        canvas_begin(width, height);
        const auto pr = 0.5 + 0.5 * sin(ek_time_now());
        drawPreloader((float) pr, width, height);
        canvas_end();

        sg_end_pass();
        sg_commit();
    }
}

void ek_app_main() {
    // ek-core systems init: +1 KB wasm
    core::setup();
    //audio::initialize();

    ek_app.config.title = "ek sample";
    ek_app.on_frame = on_frame;
    ek_app.on_ready = on_ready;
}