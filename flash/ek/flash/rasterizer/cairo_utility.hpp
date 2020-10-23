#pragma once

#include <ek/flash/doc/types.hpp>
#include "transform_model.hpp"

typedef struct _cairo cairo_t;
typedef struct _cairo_pattern cairo_pattern_t;
typedef struct _cairo_surface cairo_surface_t;

namespace ek::flash {

void clear(cairo_t* ctx);

void blit_downsample(cairo_t* ctx, cairo_surface_t* source, int w, int h, double upscale);

void set_solid_stroke(cairo_t* ctx, const solid_stroke& solid);

void set_line_cap(cairo_t* ctx, line_caps cap);

void set_line_join(cairo_t* ctx, line_joints join);

void set_solid_fill(cairo_t* context, const float4& color);

void set_blend_mode(cairo_t* ctx, blend_mode_t blend_mode);

void cairo_quadratic_curve_to(cairo_t* context, float x1, float y1, float x2, float y2);

void add_color_stops(cairo_pattern_t* pattern,
                     const std::vector<gradient_entry>& entries,
                     const float4& color_multiplier);

// https://github.com/lightspark/lightspark/blob/master/src/backends/graphics.cpp
cairo_pattern_t* create_linear_pattern(const matrix_2d& matrix);

cairo_pattern_t* create_radial_pattern(const matrix_2d& matrix);

cairo_pattern_t* create_fill_pattern(const fill_style& fill, const transform_model& transform);

void cairo_round_rectangle(cairo_t* cr, const float* values);

void cairo_oval(cairo_t* cr, const float* values);

void cairo_transform(cairo_t* cr, const matrix_2d& m);

}


