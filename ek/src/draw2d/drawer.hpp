#pragma once

#include <vector>
#include "batcher.hpp"
#include <ek/utility/common_macro.hpp>
#include <ek/math/mat3x2.hpp>
#include <graphics/program.hpp>
#include <graphics/texture.hpp>
#include <ek/assets.hpp>
#include <ek/math/circle.hpp>

namespace ek {

class drawer_t : private disable_copy_assign_t {
public:

    template<typename T>
    using stack_type = std::vector<T>;

    batcher_t batcher;

    matrix_2d matrix;
    rect_f uv{0.0f, 0.0f, 1.0f, 1.0f};
    argb32_t color_multiplier{0xFFFFFFFF};
    argb32_t color_offset{0x0};

    premultiplied_abgr32_t vertex_color_multiplier{0xFFFFFFFF};
    abgr32_t vertex_color_offset{0x0};

    drawer_t();

    ~drawer_t();

    void begin(int x, int y, int width, int height);

    void end();

    void prepare();

    void triangles(int vertex_count, int index_count);

    inline void quad(const rect_f& rc, argb32_t color = argb32_t::one) {
        quad(rc.x, rc.y, rc.width, rc.height, color);
    }

    void quad(float x, float y, float w, float h);

    void quad_rotated(float x, float y, float w, float h);

    void quad(float x, float y, float w, float h, argb32_t color);

    void quad(float x, float y, float w, float h, argb32_t c1, argb32_t c2, argb32_t c3, argb32_t c4);

    void fill_circle(const circle_f& circle, argb32_t inner_color, argb32_t outer_color, int segments);

    void write_vertex(float x, float y, float u, float v, premultiplied_abgr32_t cm, abgr32_t co);

    void write_raw_vertex(const float2& pos, const float2& tex_coord, premultiplied_abgr32_t cm, abgr32_t co);

    inline void write_index(uint16_t index) {
        *(index_memory_ptr_++) = batcher.get_vertex_index(index);
    }

    void write_indices_quad(uint16_t i0,
                            uint16_t i1,
                            uint16_t i2,
                            uint16_t i3,
                            uint16_t base_vertex = 0u);

    inline void write_indices_quad(const uint16_t base_index = 0) {
        write_indices_quad(0, 1, 2, 3, base_index);
    }

    void write_indices(const uint16_t* source, uint16_t count, uint16_t base_vertex = 0);

    void draw_indexed_triangles(const std::vector<float2>& positions, const std::vector<argb32_t>& colors,
                                const std::vector<uint16_t>& indices, const float2& offset, const float2& scale);

    void line(const float2& start, const float2& end,
              argb32_t color1, argb32_t color2,
              float lineWidth1, float lineWidth2);

    void line(const float2& start, const float2& end, argb32_t color, float lineWidth);

    void line(const float2& start, const float2& end);

    void line_arc(float x, float y, float r,
                  float angle_from, float angle_to,
                  float line_width, int segments,
                  argb32_t color_inner, argb32_t color_outer);

    /** Scissors **/

    void begin_scissors(const rect_f& scissors);

    void end_scissors();

    /** Matrix Transform **/

    drawer_t& save_matrix();

    drawer_t& save_transform();

    drawer_t& restore_transform();

    drawer_t& transform_pivot(float2 position, float rotation, float2 scale, float2 pivot) {
        matrix.translate(position.x + pivot.x, position.y + pivot.y)
                .scale(scale.x, scale.y)
                .rotate(rotation)
                .translate(-pivot.x, -pivot.y);
        return *this;
    }

    drawer_t& translate(float tx, float ty);

    drawer_t& translate(const float2& v);

    drawer_t& scale(float sx, float sy);

    drawer_t& scale(const float2& v);

    drawer_t& rotate(float radians);

    drawer_t& concat_matrix(const matrix_2d& r);

    drawer_t& restore_matrix();

    /** Color Transform **/

    drawer_t& save_color();

    drawer_t& restore_color();

    drawer_t& multiply_alpha(float alpha);

    drawer_t& multiply_color(argb32_t multiplier);

    drawer_t& combine_color(argb32_t multiplier, argb32_t offset);

    drawer_t& offset_color(argb32_t offset);

    inline premultiplied_abgr32_t calc_vertex_color_multiplier(argb32_t multiplier) {
        return (color_multiplier * multiplier).premultipliedABGR32(color_offset.a);
    }

    /** STATES **/

    drawer_t& save_canvas_rect();

    drawer_t& restore_canvas_rect();

    drawer_t& save_projection_matrix();

    drawer_t& restore_projection_matrix();

    drawer_t& save_texture_coords();

    drawer_t& set_texture_coords(float u0, float v0, float du, float dv);

    drawer_t& set_texture_coords(const rect_f& uv_rect);

    drawer_t& restore_texture_coords();

    drawer_t& save_texture();

    drawer_t& set_empty_texture();

    drawer_t& set_texture(const texture_t* texture);

    drawer_t& set_texture_region(const texture_t* texture = nullptr, const rect_f& region = rect_f::zero_one);

    drawer_t& restore_texture();

    drawer_t& save_program();

    drawer_t& restore_program();

    drawer_t& save_blend_mode();

    drawer_t& restore_blend_mode();

    void set_blend_mode(blend_mode blend_mode);

    inline const rect_f& canvas_rect() const {
        return canvas_rect_;
    }

private:
    asset_t<program_t> default_program_;
    asset_t<texture_t> default_texture_;

    rect_f canvas_rect_{};
    const texture_t* texture_{};

    stack_type<matrix_2d> matrix_stack_;
    stack_type<argb32_t> multipliers_;
    stack_type<argb32_t> offsets_;
    stack_type<rect_f> scissor_stack_;
    stack_type<const program_t*> program_stack_;
    stack_type<const texture_t*> texture_stack_;
    stack_type<blend_mode> blend_mode_stack_;
    stack_type<mat4f> projection_stack_;
    stack_type<rect_f> tex_coords_stack_;
    stack_type<rect_f> canvas_stack_;

    void* vertex_memory_ptr_ = nullptr;
    uint16_t* index_memory_ptr_ = nullptr;
};
}