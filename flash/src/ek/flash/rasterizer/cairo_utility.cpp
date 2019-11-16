#include "cairo_utility.h"
#include "transform_model.h"

#include <cairo.h>
#include <ek/flash/doc/graphic_types.h>

namespace ek::flash {

cairo_line_cap_t convert_line_cap(line_caps cap) {
    switch (cap) {
        case line_caps::none:
            return CAIRO_LINE_CAP_BUTT;
        case line_caps::round:
            return CAIRO_LINE_CAP_ROUND;
        case line_caps::square:
            return CAIRO_LINE_CAP_SQUARE;
    }
}

cairo_line_join_t convert_line_join(line_joints join) {
    switch (join) {
        case line_joints::miter:
            return CAIRO_LINE_JOIN_MITER;
        case line_joints::round:
            return CAIRO_LINE_JOIN_ROUND;
        case line_joints::bevel:
            return CAIRO_LINE_JOIN_BEVEL;
    }
}

void set_line_cap(cairo_t* ctx, line_caps cap) {
    cairo_set_line_cap(ctx, convert_line_cap(cap));
}

void set_line_join(cairo_t* ctx, line_joints join) {
    cairo_set_line_join(ctx, convert_line_join(join));
}

void set_solid_fill(cairo_t* context, const float4& color) {
    cairo_set_source_rgba(context, color.x, color.y, color.z, color.w);
}

void set_blend_mode(cairo_t* ctx, blend_mode_t blend_mode) {
    cairo_operator_t cop{cairo_operator_t::CAIRO_OPERATOR_OVER};
    switch (blend_mode) {
        case blend_mode_t::multiply:
            cop = cairo_operator_t::CAIRO_OPERATOR_MULTIPLY;
            break;
        case blend_mode_t::screen:
            cop = cairo_operator_t::CAIRO_OPERATOR_SCREEN;
            break;
        case blend_mode_t::overlay:
            cop = cairo_operator_t::CAIRO_OPERATOR_OVERLAY;
            break;
        case blend_mode_t::hardlight:
            cop = cairo_operator_t::CAIRO_OPERATOR_HARD_LIGHT;
            break;
        case blend_mode_t::normal:
            break;
        case blend_mode_t::layer:
            break;
        case blend_mode_t::last:
            break;
        case blend_mode_t::lighten:
            cop = cairo_operator_t::CAIRO_OPERATOR_LIGHTEN;
            break;
        case blend_mode_t::darken:
            cop = cairo_operator_t::CAIRO_OPERATOR_DARKEN;
            break;
        case blend_mode_t::difference:
            cop = cairo_operator_t::CAIRO_OPERATOR_DIFFERENCE;
            break;
        case blend_mode_t::add:
            cop = cairo_operator_t::CAIRO_OPERATOR_ADD;
            break;
        case blend_mode_t::subtract:
            cop = cairo_operator_t::CAIRO_OPERATOR_EXCLUSION;
            break;
        case blend_mode_t::invert:
            cop = cairo_operator_t::CAIRO_OPERATOR_XOR; // ?
            break;
        case blend_mode_t::alpha:
            cop = cairo_operator_t::CAIRO_OPERATOR_ATOP; // ?
            break;
        case blend_mode_t::erase:
            cop = cairo_operator_t::CAIRO_OPERATOR_CLEAR; // ?
            break;
    }
    cairo_set_operator(ctx, cop);
}


void cairo_quadratic_curve_to(cairo_t* context, float x1, float y1, float x2, float y2) {
    double x = 0.0;
    double y = 0.0;
    cairo_get_current_point(context, &x, &y);

    if (0.0 == x && 0.0 == y) {
        x = x1;
        y = y1;
    }

    cairo_curve_to(context, x + 2.0 / 3.0 * (x1 - x), y + 2.0 / 3.0 * (y1 - y), x2 + 2.0 / 3.0 * (x1 - x2),
                   y2 + 2.0 / 3.0 * (y1 - y2), x2, y2);
}

void add_color_stops(cairo_pattern_t* pattern,
                     const std::vector<gradient_entry>& entries,
                     const color_transform_f& color_transform) {
    for (const auto& entry: entries) {
        const auto& color = color_transform.transform(entry.color);
        cairo_pattern_add_color_stop_rgba(pattern, entry.ratio, color.x, color.y, color.z, color.w);
    }
}

// https://github.com/lightspark/lightspark/blob/master/src/backends/graphics.cpp
cairo_pattern_t* create_linear_pattern(const matrix_2d& matrix) {
    const auto p0 = matrix.transform(-819.2f, 0.0f);
    const auto p1 = matrix.transform(819.2f, 0.0f);
    return cairo_pattern_create_linear(p0.x, p0.y, p1.x, p1.y);
}

cairo_pattern_t* create_radial_pattern(const matrix_2d& matrix) {
    const auto p0 = matrix.transform(0.0f, 0.0f);
    const auto p1 = matrix.transform(819.2f, 0.0f);
    const auto radius = length(p1 - p0);
    return cairo_pattern_create_radial(p0.x, p0.y, 0.0, p0.x, p0.y, radius);
}

void set_solid_stroke(cairo_t* ctx, const solid_stroke& solid) {
    cairo_set_line_width(ctx, solid.weight);
    set_line_cap(ctx, solid.caps);
    set_line_join(ctx, solid.joints);
    cairo_set_miter_limit(ctx, solid.miterLimit);
}

cairo_pattern_t* create_fill_pattern(const fill_style& fill, const transform_model& transform) {
    cairo_pattern_t* pattern = nullptr;

    switch (fill.type) {
        case fill_type::linear:
            pattern = create_linear_pattern(transform.matrix * fill.matrix);
            add_color_stops(pattern, fill.entries, transform.color);
            break;
        case fill_type::radial:
            pattern = create_radial_pattern(transform.matrix * fill.matrix);
            add_color_stops(pattern, fill.entries, transform.color);
            break;
        default:
            break;
    }

    if (pattern) {
        switch (fill.spreadMethod) {
            case spread_method::extend:
                cairo_pattern_set_extend(pattern, CAIRO_EXTEND_PAD);
                break;
            case spread_method::reflect:
                cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REFLECT);
                break;
            case spread_method::repeat:
                cairo_pattern_set_extend(pattern, CAIRO_EXTEND_REPEAT);
                break;
        }

        cairo_pattern_set_filter(pattern, CAIRO_FILTER_BEST);
    }

    return pattern;
}


}