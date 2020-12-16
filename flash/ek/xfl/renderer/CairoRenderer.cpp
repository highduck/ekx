#include "CairoRenderer.hpp"

#include "RenderCommand.hpp"
#include "CairoHelpers.hpp"
#include <cairo.h>

namespace ek::xfl {

CairoRenderer::CairoRenderer(cairo_t* ctx) : ctx_{ctx} {

}

void CairoRenderer::set_transform(const TransformModel& transform) {
    transform_ = transform;
    set_blend_mode(ctx_, transform.blendMode);
}

void CairoRenderer::execute(const RenderCommand& cmd) {
    using Op = RenderCommand::Operation;

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

        case Op::rectangle:
            cairo_save(ctx_);
            cairo_transform(ctx_, transform_.matrix);
            cairo_new_path(ctx_);
            cairo_round_rectangle(ctx_, cmd.v);
            cairo_restore(ctx_);

            stroke_flag_ = cmd.stroke != nullptr;
            stroke_style_ = cmd.stroke;
            fill_flag_ = cmd.fill != nullptr;
            fill_style_ = cmd.fill;
            paint();
            break;

        case Op::oval:
            cairo_save(ctx_);
            cairo_transform(ctx_, transform_.matrix);
            cairo_new_path(ctx_);
            cairo_oval(ctx_, cmd.v);
            cairo_restore(ctx_);

            stroke_flag_ = cmd.stroke != nullptr;
            stroke_style_ = cmd.stroke;
            fill_flag_ = cmd.fill != nullptr;
            fill_style_ = cmd.fill;
            paint();
            break;
        case Op::bitmap:
            draw_bitmap(cmd.bitmap);
            break;
    }
}

void CairoRenderer::open() {
    if (!open_flag_) {
        cairo_new_path(ctx_);
        open_flag_ = true;
    }
}

void CairoRenderer::fill() {
    auto pattern = set_fill_style(ctx_, *fill_style_, transform_);

    cairo_set_fill_rule(ctx_, CAIRO_FILL_RULE_EVEN_ODD);
    cairo_fill_preserve(ctx_);

    pattern.destroy();
}

void CairoRenderer::paint() {
    if (fill_flag_ && fill_style_) {
        // set fill style color
        fill();
    }

    if (stroke_flag_ && stroke_style_) {
        stroke();
    }

    stroke_flag_ = false;
    fill_flag_ = false;
}

void CairoRenderer::close() {
    if (open_flag_) {
        if (fill_flag_) {
            cairo_close_path(ctx_);
        }
        paint();
        open_flag_ = false;
    }
}

void CairoRenderer::draw_bitmap(const BitmapData* bitmap) {
    const int sx = 0;
    const int sy = 0;
    const int sw = bitmap->width;
    const int sh = bitmap->height;

    auto source_surface = cairo_image_surface_create_for_data(
            const_cast<uint8_t*>(bitmap->data.data()),
            CAIRO_FORMAT_ARGB32, sw, sh, sw * 4);
    auto source_pattern = cairo_pattern_create_for_surface(source_surface);

    cairo_save(ctx_);
    cairo_transform(ctx_, transform_.matrix);
    cairo_rectangle(ctx_, sx, sy, sw, sh);
    cairo_restore(ctx_);

    cairo_set_source(ctx_, source_pattern);
    cairo_fill(ctx_);
    cairo_pattern_destroy(source_pattern);
    cairo_surface_destroy(source_surface);
}

void CairoRenderer::stroke() {
    auto pattern = set_fill_style(ctx_, stroke_style_->fill, transform_);
    set_stroke_style(ctx_, *stroke_style_);

    cairo_stroke_preserve(ctx_);

    pattern.destroy();
}

}
