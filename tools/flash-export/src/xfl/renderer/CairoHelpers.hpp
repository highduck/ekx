#pragma once

#include "../types.hpp"
#include <ek/ds/Array.hpp>

typedef struct _cairo cairo_t;
typedef struct _cairo_pattern cairo_pattern_t;
typedef struct _cairo_surface cairo_surface_t;

namespace ek::xfl {

struct fill_pattern_data_t {
    cairo_pattern_t* pattern = nullptr;
    cairo_surface_t* surface = nullptr;

    void destroy();
};

void destroy_pattern_data(cairo_t* cr, fill_pattern_data_t& pattern);

void clear(cairo_t* ctx);

void blit_downsample(cairo_t* ctx, cairo_surface_t* source, int w, int h, double upscale);

void set_stroke_style(cairo_t* ctx, const StrokeStyle& stroke);

fill_pattern_data_t set_fill_style(cairo_t* cr, const FillStyle& fill, const TransformModel& transform);

void set_line_cap(cairo_t* ctx, LineCaps cap);

void set_line_join(cairo_t* ctx, LineJoints join);

void set_solid_fill(cairo_t* context, vec4_t color);

void set_blend_mode(cairo_t* ctx, BlendMode blend_mode);

void cairo_quadratic_curve_to(cairo_t* context, float x1, float y1, float x2, float y2);

void add_color_stops(cairo_pattern_t* pattern,
                     const Array<GradientEntry>& entries,
                     vec4_t color_multiplier);

// https://github.com/lightspark/lightspark/blob/master/src/backends/graphics.cpp
cairo_pattern_t* create_linear_pattern(const mat3x2_t& matrix);

cairo_pattern_t* create_radial_pattern(const mat3x2_t& matrix);

fill_pattern_data_t create_fill_pattern(const FillStyle& fill, const TransformModel& transform);

void cairo_round_rectangle(cairo_t* cr, const double* values);

void cairo_oval(cairo_t* cr, const double* values);

void cairo_transform(cairo_t* cr, const mat3x2_t& m);

}


