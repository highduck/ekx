#pragma once

#include <ek/util/Type.hpp>
#include <ek/gfx.h>
#include <ek/math.h>

enum {
    R_SPRITE_EMPTY = 1
};

enum sprite_flags_t {
    SPRITE_LOADED = 1,
    SPRITE_ROTATED = 2
};

typedef struct sprite_t {
    uint32_t state;
    R(sg_image) image_id;
    rect_t rect;
    rect_t tex;
} sprite_t;

struct res_sprite {
    string_hash_t names[256];
    sprite_t data[256];
    rr_man_t rr;
};

extern struct res_sprite res_sprite;

void setup_res_sprite(void);

#define R_SPRITE(name) REF_NAME(res_sprite, name)

namespace ek {

void draw(const sprite_t* sprite);

void draw(const sprite_t* sprite, rect_t rc);

void draw_grid(const sprite_t* sprite, rect_t grid, rect_t target);

[[nodiscard]] bool hit_test(const sprite_t* sprite, vec2_t position);

[[nodiscard]] bool select(const sprite_t* sprite);

}

