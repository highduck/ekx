#pragma once

#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Type.hpp>
#include <ek/gfx.h>
#include <ek/math.h>

namespace ek {

enum sprite_flags_t {
    SPRITE_LOADED = 1,
    SPRITE_ROTATED = 2
};

struct Sprite {
    uint32_t state = 0;
    REF_TO(sg_image) image_id = 0;
    rect_t rect = rect_01();
    rect_t tex = rect_01();
};

struct res_sprite {
    string_hash_t names[256];
    Sprite data[256];
    rr_man_t rr;
};

extern struct res_sprite res_sprite;

void setup_res_sprite(void);

void draw(const Sprite* sprite);

void draw(const Sprite* sprite, rect_t rc);

void draw_grid(const Sprite* sprite, rect_t grid, rect_t target);

[[nodiscard]] bool hit_test(const Sprite* sprite, vec2_t position);

[[nodiscard]] bool select(const Sprite* sprite);

}

