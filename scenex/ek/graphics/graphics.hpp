#pragma once

#include <cstdint>
#include "blending.hpp"

namespace ek::graphics {

class buffer_t;

class texture_t;

class render_target_t;

class program_t;

struct vertex_decl_t;

void init();

void begin();

void clear(float r, float g, float b, float a);

void viewport(int x, int y, int wight, int height);

void viewport();

void set_blend_mode(const blend_mode& blending);

void set_scissors(int x, int y, int width, int height);

void set_scissors();

void get_pixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* out_buffer);

void draw_triangles(uint32_t indices_count);

}