#include <ek/app.h>
#include <ek/core.hpp>
#include <ek/gfx.h>
#include <ek/canvas.h>
#include <ek/rnd.h>
#include <ek/math.h>
#include <ek/bitset.h>
#include <ek/time.h>

using namespace ek;

struct random_estimator_t {
    uint64_t* bitset;
    uint64_t* bitset_occupy;
    uint32_t* pixels;
    sg_image image;
    int bitset_size;
    int image_size;
};

void estimator_init(random_estimator_t* estimator) {
    memset(estimator, 0, sizeof(random_estimator_t));
    const size_t side = 1 << 15; // 2 ^ 15
    estimator->bitset_size = side;
    estimator->image_size = 1024;

    const uint32_t bitset_bytes = ek_bitset_byte_size(side * side);
    const uint32_t image_data_size = estimator->image_size * estimator->image_size * sizeof(uint32_t);
    char* mem = (char*) calloc(1, bitset_bytes + bitset_bytes + image_data_size);
    estimator->bitset = (uint64_t*) mem;
    estimator->bitset_occupy = (uint64_t*) (mem + bitset_bytes);
    estimator->pixels = (uint32_t*) (mem + bitset_bytes + bitset_bytes);

    sg_image_desc desc{};
    desc.type = SG_IMAGETYPE_2D;
    desc.width = estimator->image_size;
    desc.height = estimator->image_size;
    desc.usage = SG_USAGE_DYNAMIC;
    desc.pixel_format = SG_PIXELFORMAT_RGBA8;
    estimator->image = sg_make_image(desc);
}

void estimator_set(random_estimator_t* estimator, uint32_t x, uint32_t y) {
    const uint32_t bp = y * estimator->bitset_size + x;
    ek_bitset_flip(estimator->bitset, bp);
    ek_bitset_set(estimator->bitset_occupy, bp);
}

void estimator_update_pixels(random_estimator_t* estimator) {
    int size = estimator->image_size;
    for (int cy = 0; cy < size; ++cy) {
        for (int cx = 0; cx < size; ++cx) {
            const uint32_t ip = size * cy + cx;
            const uint32_t bp = estimator->bitset_size * cy + cx;
            if (ek_bitset_get(estimator->bitset_occupy, bp)) {
                const bool odd = ek_bitset_get(estimator->bitset, bp);
                estimator->pixels[ip] = odd ? 0xFF000000 : 0xFFFFFFFF;
            } else {
                estimator->pixels[ip] = 0xFF777777;
            }
        }
    }
    const uint32_t image_data_size = estimator->image_size * estimator->image_size * 4;
    ek_gfx_update_image_0(estimator->image, estimator->pixels, image_data_size);
}


random_estimator_t estimator;

void on_ready() {
    ek_gfx_setup(128);
    canvas_setup();

    estimator_init(&estimator);
}

void on_frame() {
    const auto width = ek_app.viewport.width;
    const auto height = ek_app.viewport.height;
    if (width > 0 && height > 0) {
        static sg_pass_action pass_action{};
        pass_action.colors[0].action = SG_ACTION_CLEAR;
        const vec4_t fillColor = vec4_color(ARGB(ek_app.config.background_color));
        pass_action.colors[0].value.r = fillColor.x;
        pass_action.colors[0].value.g = fillColor.y;
        pass_action.colors[0].value.b = fillColor.z;
        pass_action.colors[0].value.a = 1.0f;
        sg_begin_default_pass(&pass_action, (int) width, (int) height);

        for (int i = 0; i < 5000000; ++i) {
            const uint32_t cx = (uint32_t) random_range_f(0, (float) estimator.bitset_size);
            const uint32_t cy = (uint32_t) random_range_f(0, (float) estimator.bitset_size);
//                uint32_t cx = (uint32_t) (rand_default.next() % estimator.bitset_size);
//                uint32_t cy = (uint32_t) (rand_default.next() % estimator.bitset_size);
//                const uint32_t cx = rand() % bitset_segments;
//                const uint32_t cy = rand() % bitset_segments;
            estimator_set(&estimator, cx, cy);
        }

        estimator_update_pixels(&estimator);

        canvas_new_frame();
        canvas_begin(width, height);

        canvas_set_image_region(estimator.image, rect_01());
        canvas_quad(0, 0, estimator.image_size, estimator.image_size);

        canvas_end();

        sg_end_pass();
        sg_commit();
    }
}

void ek_app_main() {
    core::setup();
    srand(ek_time_seed32());

    ek_app.config.title = "rnd test";
    ek_app.on_frame = on_frame;
    ek_app.on_ready = on_ready;
}