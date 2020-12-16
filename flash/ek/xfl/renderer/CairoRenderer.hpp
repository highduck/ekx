#pragma once

#include "transform_model.hpp"

typedef struct _cairo cairo_t;
typedef struct _cairo_pattern cairo_pattern_t;
typedef struct _cairo_surface cairo_surface_t;

namespace ek::xfl {

struct render_command;
struct fill_style;
struct stroke_style;
struct BitmapData;

class CairoRenderer {
public:
    explicit CairoRenderer(cairo_t* ctx);

    void execute(const render_command& cmd);

    void set_transform(const transform_model& transform);

    void draw_bitmap(const BitmapData* bitmap);

private:
    cairo_t* ctx_;

    transform_model transform_;

    bool fill_flag_ = false;
    bool stroke_flag_ = false;
    bool open_flag_ = false;

    const fill_style* fill_style_ = nullptr;
    const stroke_style* stroke_style_ = nullptr;

    void open();

    void fill();
    void stroke();

    void close();

    void paint();
};

}
