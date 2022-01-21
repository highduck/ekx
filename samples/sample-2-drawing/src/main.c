#include <ek/app.h>
//#include <ek/audio/audio.hpp>
#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/time.h>
#include <ek/math.h>
#include <ek/log.h>
#include "config/build_info.h"

void drawPreloader(float progress, float zoneWidth, float zoneHeight) {
    canvas_set_empty_image();
    float pad = 40.0f;
    float w = zoneWidth - pad * 2.0f;
    float h = 16.0f;
    float y = (zoneHeight - h) * 0.5f + h;

    canvas_quad_color(pad, y, w, h, COLOR_WHITE);
    canvas_quad_color(pad + 2, y + 2, w - 4, h - 4, COLOR_BLACK);
    canvas_quad_color(pad + 4, y + 4, (w - 8) * progress, h - 8, COLOR_WHITE);

    float sz = zoneWidth < zoneHeight ? zoneWidth : zoneHeight;
    float cx = zoneWidth / 2.0f;
    float cy = zoneHeight / 2.0f;
    float sh = sz / 16.0f;
    float sw = sh * 3;
    const float time = (float) ek_time_now();
    for (int i = 0; i < 7; ++i) {
        float r = ((float) i / 7) * 1.5f + time;
        float speed = (0.5f + 0.5f * sinf(r * 2 + 1));
        r = r + 0.5f * speed;
        float ox = sinf(r * 3.14f);
        float oy = sinf(r * 3.14f * 2 + 3.14f);
        float R = (sh / 10.0f) *
                  (1.8f - 0.33f * speed - 0.33f * ((cosf(r * 3.14f) + 2.0f * cosf(r * 3.14f * 2 + 3.14f))));
        vec3_t circ = vec3(cx + ox * sw, cy - 2.0f * sh + oy * sh, R);
        canvas_fill_circle(circ, COLOR_WHITE, COLOR_WHITE, 16);
    }
}

void on_ready() {
    ek_gfx_setup(128);
    canvas_setup();
}

void on_frame() {
    const float width = ek_app.viewport.width;
    const float height = ek_app.viewport.height;
    if (width > 0 && height > 0) {
        static sg_pass_action pass_action = {};
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        const vec4_t fillColor = vec4_color(ARGB(ek_app.config.background_color));
        pass_action.colors[0].value.r = fillColor.x;
        pass_action.colors[0].value.g = fillColor.y;
        pass_action.colors[0].value.b = fillColor.z;
        pass_action.colors[0].value.a = 1.0f;
        sg_begin_default_pass(&pass_action, (int) width, (int) height);

        canvas_new_frame();
        canvas_begin(width, height);
        const float pr = 0.5f + 0.5f * sinf((float)ek_time_now());
        drawPreloader(pr, width, height);
        canvas_end();

        sg_end_pass();
        sg_commit();
    }
}

void ek_app_main() {
    log_init();
    ek_time_init();

    log_info("Name: sample-2-drawing");
    log_info("Version: %s #%s", APP_VERSION_NAME, APP_VERSION_CODE);

    ek_app.config.title = "ek sample";
    ek_app.on_frame = on_frame;
    ek_app.on_ready = on_ready;
}