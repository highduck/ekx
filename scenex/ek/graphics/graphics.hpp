#pragma once

#include <ek/util/common_macro.hpp>
#include <cstdint>

namespace ek {

struct blend_mode;

class graphics_t : private disable_copy_assign_t {
    friend class texture_t;

    friend class batcher_t;

    friend class program_t;

public:

    graphics_t();

    ~graphics_t();

    void begin();

    void clear(float r, float g, float b, float a);

    void viewport(int x, int y, int wight, int height);

    void viewport();

    void set_blend_mode(const blend_mode& blend_mode);

    void set_scissors(int x, int y, int width, int height);

    void set_scissors();

    void get_pixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* out_buffer);

    void draw_triangles(uint32_t indices_count);
};

}