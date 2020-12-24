#pragma once

#include <vector>
#include "batcher.hpp"
#include <ek/util/common_macro.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/graphics/Helpers.hpp>
#include <ek/util/Res.hpp>
#include <ek/math/circle.hpp>
#include <ek/math/box.hpp>

namespace ek::draw2d {

struct drawing_state {

    Res<graphics::Shader> default_program{"draw2d"};
    Res<graphics::Texture> default_texture{"empty"};

    const graphics::Texture* texture{};
    const graphics::Shader* program{};
    matrix_2d matrix{};
    mat4f mvp{};
    rect_f uv{0.0f, 0.0f, 1.0f, 1.0f};
    ColorMod32 color{};
    rect_f scissors{};
    const graphics::Texture* renderTarget = nullptr;
    bool active = false;

public:

    std::vector<matrix_2d> matrix_stack_;
    std::vector<ColorMod32> colors_;
    std::vector<rect_f> scissorsStack_;
    std::vector<const graphics::Shader*> program_stack_;
    std::vector<const graphics::Texture*> texture_stack_;
    std::vector<rect_f> tex_coords_stack_;

    enum CheckFlags : uint8_t {
        Check_Scissors = 1,
        Check_Shader = 4,
        Check_Texture = 8
    };
    uint8_t checkFlags = 0;

    /** Scissors **/

    void pushClipRect(const rect_f& rc);

    drawing_state& saveScissors();

    void setScissors(const rect_f& rc);

    drawing_state& popClipRect();

    /** Matrix Transform **/

    drawing_state& save_matrix();

    drawing_state& save_transform();

    drawing_state& restore_transform();

    drawing_state& transform_pivot(float2 position, float rotation, float2 scale, float2 pivot) {
        matrix.translate(position.x + pivot.x, position.y + pivot.y)
                .scale(scale.x, scale.y)
                .rotate(rotation)
                .translate(-pivot.x, -pivot.y);
        return *this;
    }

    drawing_state& translate(float tx, float ty);

    drawing_state& translate(const float2& v);

    drawing_state& scale(float sx, float sy);

    drawing_state& scale(const float2& v);

    drawing_state& rotate(float radians);

    drawing_state& concat(const matrix_2d& r);

    drawing_state& restore_matrix();

    /** Color Transform **/

    drawing_state& save_color();

    drawing_state& restore_color();

    drawing_state& scaleAlpha(float alpha);

    drawing_state& scaleColor(abgr32_t multiplier);

    drawing_state& concat(abgr32_t scale, abgr32_t offset);

    drawing_state& concat(ColorMod32 color);

    drawing_state& offset_color(abgr32_t offset);

    drawing_state& save_texture_coords();

    drawing_state& set_texture_coords(float u0, float v0, float du, float dv);

    drawing_state& set_texture_coords(const rect_f& uv_rect);

    drawing_state& restore_texture_coords();

    drawing_state& save_texture();

    drawing_state& set_empty_texture();

    drawing_state& set_texture(const graphics::Texture* texture);

    drawing_state& set_texture_region(const graphics::Texture* texture = nullptr,
                                      const rect_f& region = rect_f::zero_one);

    drawing_state& restore_texture();

    drawing_state& pushProgram(const char* id);

    drawing_state& setProgram(const graphics::Shader* program_);

    drawing_state& saveProgram();

    drawing_state& restoreProgram();

    // do extra checking and clear states stack
    void finish();
};

extern drawing_state state;

void init();

void beginNewFrame();

void begin(rect_i viewport, const matrix_2d& view = matrix_2d{}, const graphics::Texture* renderTarget = nullptr);

uint32_t getBatchingUsedMemory();

void end();

void endFrame();

void write_index(uint16_t index);

FrameStats getDrawStats();

void triangles(int vertex_count, int index_count);

void quad(float x, float y, float w, float h);

void quad_rotated(float x, float y, float w, float h);

void quad(float x, float y, float w, float h, abgr32_t color);

void quad(float x, float y, float w, float h, abgr32_t c1, abgr32_t c2, abgr32_t c3, abgr32_t c4);

inline void quad(const rect_f& rc, abgr32_t color = abgr32_t::one) {
    quad(rc.x, rc.y, rc.width, rc.height, color);
}

void fill_circle(const circle_f& circle, abgr32_t inner_color, abgr32_t outer_color, int segments);

void write_vertex(float x, float y, float u, float v, abgr32_t cm, abgr32_t co);

void write_raw_vertex(const float2& pos, const float2& tex_coord, abgr32_t cm, abgr32_t co);

void write_indices_quad(uint16_t i0,
                        uint16_t i1,
                        uint16_t i2,
                        uint16_t i3,
                        uint16_t base_vertex = 0u);

inline void write_indices_quad(const uint16_t base_index = 0) {
    write_indices_quad(0, 1, 2, 3, base_index);
}

Batcher* getBatcher();

void write_indices(const uint16_t* source, uint16_t count, uint16_t base_vertex = 0);

void draw_indexed_triangles(const std::vector<float2>& positions,
                            const std::vector<abgr32_t>& colors,
                            const std::vector<uint16_t>& indices,
                            const float2& offset,
                            const float2& scale);

void line(const float2& start, const float2& end,
          abgr32_t color1, abgr32_t color2,
          float lineWidth1, float lineWidth2);

void line(const float2& start, const float2& end, abgr32_t color = abgr32_t::one, float lineWidth = 1.0f);

void line_arc(float x, float y, float r,
              float angle_from, float angle_to,
              float line_width, int segments,
              abgr32_t color_inner, abgr32_t color_outer);

void strokeRect(const rect_f& rc, abgr32_t color, float lineWidth);

}