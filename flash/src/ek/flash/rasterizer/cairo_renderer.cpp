#include "cairo_renderer.h"

#include "render_command.h"
#include "cairo_utility.h"
#include <ek/flash/doc/flash_file.h>
#include <cairo.h>

namespace ek::flash {

cairo_renderer::cairo_renderer(cairo_t* ctx)
        : ctx_{ctx} {

}

void cairo_renderer::set_transform(const transform_model& transform) {
    transform_ = transform;
    set_blend_mode(ctx_, transform.blend_mode);
}

void cairo_renderer::execute(const render_command& cmd) {
    using Op = render_command::operation;

    switch (cmd.op) {
        case Op::line_style_setup: {
            if (open_flag_ && stroke_flag_) {
                close();
            }
            open();
            stroke_flag_ = true;
            stroke_style_ = cmd.stroke;
        }
            break;
        case Op::line_style_reset:
            close();
            stroke_flag_ = false;
            break;
        case Op::fill_end:
            close();
            fill_flag_ = false;
            break;
        case Op::fill_begin: {
            if (open_flag_ && fill_flag_) {
                close();
            }
            open();

            fill_flag_ = true;
            fill_style_ = cmd.fill;
        }
            break;
        case Op::line_to:
            cairo_line_to(ctx_, cmd.v[0], cmd.v[1]);
            break;

        case Op::curve_to:
            cairo_quadratic_curve_to(ctx_, cmd.v[0], cmd.v[1], cmd.v[2], cmd.v[3]);
            break;

        case Op::move_to:
            cairo_move_to(ctx_, cmd.v[0], cmd.v[1]);
            break;
    }
}

void cairo_renderer::open() {
    if (!open_flag_) {
        cairo_new_path(ctx_);
        open_flag_ = true;
    }
}

void cairo_renderer::fill() {
    cairo_pattern_t* pattern = nullptr;
    if (fill_style_) {
        if (fill_style_->type == fill_type::solid) {
            set_solid_fill(ctx_, transform_.color.transform(fill_style_->entries[0].color));
        } else {
            pattern = create_fill_pattern(*fill_style_, transform_);
        }
    }

    if (pattern) {
        cairo_set_source(ctx_, pattern);
    }

    cairo_set_fill_rule(ctx_, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_fill_preserve(ctx_);

    if (pattern) {
        cairo_pattern_destroy(pattern);
    }
}

void cairo_renderer::close() {
    if (open_flag_) {
        cairo_close_path(ctx_);

        if (fill_flag_) {
            // set fill style color
            fill();
        }

        if (stroke_flag_ && stroke_style_) {
            const auto& solid = stroke_style_->solid;
            set_solid_stroke(ctx_, stroke_style_->solid);
            float4 c = transform_.color.transform(solid.fill);
            cairo_set_source_rgba(ctx_,
                                  c.x,
                                  c.y,
                                  c.z,
                                  c.w);
            cairo_stroke_preserve(ctx_);
        }
//        else if (fill_flag_) {
//            static solid_stroke hairline{};
//            hairline.fill = transform_.color.transform(fill_style_->entries[0].color);
//            hairline.weight = 0.15f;
//            set_solid_stroke(ctx_, hairline);
//            cairo_stroke(ctx_);
//        }

        stroke_flag_ = false;
        fill_flag_ = false;
        open_flag_ = false;
    }
}

void cairo_renderer::draw_bitmap(const bitmap_t* bitmap) {
    const int sx = 0;
    const int sy = 0;
    const int sw = bitmap->width;
    const int sh = bitmap->height;

    auto source_surface = cairo_image_surface_create_for_data(bitmap->data->data(),
                                                              CAIRO_FORMAT_ARGB32,
                                                              sw,
                                                              sh,
                                                              sw * 4);
    auto source_pattern = cairo_pattern_create_for_surface(source_surface);

    cairo_save(ctx_);

    cairo_matrix_t transform_matrix;
    transform_matrix.xx = transform_.matrix.a;
    transform_matrix.yx = transform_.matrix.b;
    transform_matrix.xy = transform_.matrix.c;
    transform_matrix.yy = transform_.matrix.d;
    transform_matrix.x0 = transform_.matrix.tx;
    transform_matrix.y0 = transform_.matrix.ty;
    cairo_transform(ctx_, &transform_matrix);

    cairo_set_source(ctx_, source_pattern);

    cairo_rectangle(ctx_, sx, sy, sw, sh);
    cairo_fill(ctx_);

    cairo_restore(ctx_);

    cairo_pattern_destroy(source_pattern);
    cairo_surface_destroy(source_surface);
}

}
