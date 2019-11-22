#include "drawer.hpp"

#include <ek/locator.hpp>
#include <ek/utility/common_macro.hpp>
#include <graphics/gl_def.hpp>
#include <ek/math/matrix_camera.hpp>

namespace ek {

drawer_t::drawer_t()
        : batcher{resolve<graphics_t>()},
          default_program_{"2d"},
          default_texture_{"empty"} {

    assert_created_once<drawer_t>();

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_CULL_FACE);
}

drawer_t::~drawer_t() = default;

void drawer_t::begin(int x, int y, int width, int height) {
    batcher.begin();

    texture_ = default_texture_.get();

//        batcher.states.backbuffer_height = height;
    batcher.states.clear();

    batcher.states.set_program(default_program_.get());

    batcher.states.set_transform(mat4f{});
    batcher.states.set_projection(ortho_2d<float>(x, y, width, height));

    batcher.states.set_blend_mode(blend_mode::premultiplied);
    batcher.states.set_texture(texture_);
}

void drawer_t::end() {
    batcher.states.apply();
    batcher.flush();

    glBindBuffer(GL_ARRAY_BUFFER, (GLuint) 0u);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u);
    glBindTexture(GL_TEXTURE_2D, 0u);
    glUseProgram(0u);

    // debug checks
    assert(scissor_stack_.empty());
    assert(matrix_stack_.empty());
    assert(multipliers_.empty());
    assert(program_stack_.empty());
    assert(texture_stack_.empty());
    assert(blend_mode_stack_.empty());
    assert(projection_stack_.empty());
    assert(tex_coords_stack_.empty());
    assert(canvas_stack_.empty());

    scissor_stack_.clear();
    matrix_stack_.clear();
    multipliers_.clear();
    program_stack_.clear();
    texture_stack_.clear();
    blend_mode_stack_.clear();
    projection_stack_.clear();
    tex_coords_stack_.clear();
    canvas_stack_.clear();
}

void drawer_t::prepare() {
    vertex_color_multiplier = color_multiplier.premultipliedABGR32(color_offset.a);
    // for offset: delete alpha, flip R vs B channels
    vertex_color_offset = color_offset.bgr();
}

void drawer_t::triangles(int vertex_count, int index_count) {
    batcher.states.set_texture(texture_);
    batcher.alloc_triangles(vertex_count, index_count);
    vertex_memory_ptr_ = batcher.vertex_memory_ptr();
    index_memory_ptr_ = batcher.index_memory_ptr();
}

void drawer_t::quad(float x, float y, float w, float h) {
    prepare();

    triangles(4, 6);

    const auto cm = vertex_color_multiplier;
    const auto co = vertex_color_offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void drawer_t::quad(float x, float y, float w, float h, argb32_t color) {
    prepare();

    triangles(4, 6);

    const auto cm = calc_vertex_color_multiplier(color);
    const auto co = vertex_color_offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void drawer_t::quad(float x, float y, float w, float h, argb32_t c1, argb32_t c2, argb32_t c3, argb32_t c4) {
    prepare();

    triangles(4, 6);

    const auto co = vertex_color_offset;
    write_vertex(x, y, 0, 0.0f, calc_vertex_color_multiplier(c1), co);
    write_vertex(x + w, y, 1.0f, 0.0f, calc_vertex_color_multiplier(c2), co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, calc_vertex_color_multiplier(c3), co);
    write_vertex(x, y + h, 0.0f, 1.0f, calc_vertex_color_multiplier(c4), co);

    write_indices_quad();
}

void drawer_t::quad_rotated(float x, float y, float w, float h) {
    prepare();

    triangles(4, 6);

    const auto cm = vertex_color_multiplier;
    const auto co = vertex_color_offset;
    write_vertex(x, y, 0, 1, cm, co);
    write_vertex(x + w, y, 0, 0, cm, co);
    write_vertex(x + w, y + h, 1, 0, cm, co);
    write_vertex(x, y + h, 1, 1, cm, co);

    write_indices_quad();
}

void drawer_t::fill_circle(const circle_f& circle, argb32_t inner_color, argb32_t outer_color, int segments) {
    prepare();
    triangles(1 + segments, 3 * segments);

    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    auto inner_cm = calc_vertex_color_multiplier(inner_color);
    auto outer_cm = calc_vertex_color_multiplier(outer_color);
    write_vertex(x, y, 0.0f, 0.0f, inner_cm, vertex_color_offset);

    const float da = math::pi2 / segments;
    float a = 0.0f;
    while (a < math::pi2) {
        write_vertex(x + r * cosf(a), y + r * sinf(a), 1, 1, outer_cm, vertex_color_offset);
        a += da;
    }

    const uint16_t end = segments - 1;
    for (uint16_t i = 0; i < end; ++i) {
        write_index(0u);
        write_index(i + 1u);
        write_index(i + 2u);
    }
    write_index(0u);
    write_index(segments);
    write_index(1u);
}

void drawer_t::write_vertex(float x, float y, float u, float v, premultiplied_abgr32_t cm, abgr32_t co) {
    auto* ptr = (vertex_2d*) vertex_memory_ptr_;

    // could be cached before draw2d
    const matrix_2d& m = matrix;

    ptr->position.x = x * m.a + y * m.c + m.tx;
    ptr->position.y = x * m.b + y * m.d + m.ty;
    ptr->uv.x = uv.x + u * uv.width;
    ptr->uv.y = uv.y + v * uv.height;
    ptr->cm = cm;
    ptr->co = co;
    ++ptr;

    vertex_memory_ptr_ = (uint8_t*) ptr;
}

void drawer_t::write_raw_vertex(const float2& pos, const float2& tex_coord, premultiplied_abgr32_t cm, abgr32_t co) {
    auto* ptr = (vertex_2d*) vertex_memory_ptr_;
    ptr->position = pos;
    ptr->uv = tex_coord;
    ptr->cm = cm;
    ptr->co = co;
    ++ptr;
    vertex_memory_ptr_ = (uint8_t*) ptr;
}

void drawer_t::write_indices_quad(const uint16_t i0,
                                  const uint16_t i1,
                                  const uint16_t i2,
                                  const uint16_t i3,
                                  const uint16_t base_vertex) {
    const uint16_t index = batcher.get_vertex_index(base_vertex);
    *(index_memory_ptr_++) = index + i0;
    *(index_memory_ptr_++) = index + i1;
    *(index_memory_ptr_++) = index + i2;
    *(index_memory_ptr_++) = index + i2;
    *(index_memory_ptr_++) = index + i3;
    *(index_memory_ptr_++) = index + i0;
}

void drawer_t::write_indices(const uint16_t* source,
                             uint16_t count,
                             uint16_t base_vertex) {
    const uint16_t index = batcher.get_vertex_index(base_vertex);
    for (int i = 0; i < count; ++i) {
        *index_memory_ptr_ = (*source) + index;
        ++index_memory_ptr_;
        ++source;
    }
}

/** Scissors **/

void drawer_t::begin_scissors(const rect_f& scissors) {
    size_t size = scissor_stack_.size();
    rect_f clamped_rect = scissors;
    if (size > 0) {
        // TODO: apply clamped scissors hierarchy (not clamped alternatives)
        clamped_rect = clamp_bounds(clamped_rect, scissor_stack_.back());
    }

    batcher.flush();
    scissor_stack_.push_back(clamped_rect);

    // TODO: next scissors to avoid pre-flush
    batcher.states.set_scissors(clamped_rect);
}

void drawer_t::end_scissors() {
    batcher.flush();
    scissor_stack_.pop_back();
    // TODO: next scissors to avoid pre-flush
    if (scissor_stack_.empty()) {
        batcher.states.disable_scissors();
    } else {
        batcher.states.set_scissors(scissor_stack_.back());
    }
}

/** Matrix Transform **/

drawer_t& drawer_t::save_matrix() {
    matrix_stack_.push_back(matrix);
    return *this;
}

drawer_t& drawer_t::save_transform() {
    save_matrix();
    save_color();
    return *this;
}

drawer_t& drawer_t::restore_transform() {
    restore_matrix();
    restore_color();
    return *this;
}

drawer_t& drawer_t::translate(float tx, float ty) {
    matrix.translate(tx, ty);
    return *this;
}

drawer_t& drawer_t::translate(const float2& v) {
    matrix.translate(v);
    return *this;
}

drawer_t& drawer_t::scale(float sx, float sy) {
    matrix.scale(sx, sy);
    return *this;
}

drawer_t& drawer_t::scale(const float2& v) {
    matrix.scale(v);
    return *this;
}

drawer_t& drawer_t::rotate(float radians) {
    matrix.rotate(radians);
    return *this;
}

drawer_t& drawer_t::concat_matrix(const matrix_2d& r) {
    matrix = matrix * r;
    return *this;
}

drawer_t& drawer_t::restore_matrix() {
    matrix = matrix_stack_.back();
    matrix_stack_.pop_back();
    return *this;
}

/** Color Transform **/

drawer_t& drawer_t::save_color() {
    multipliers_.push_back(color_multiplier);
    offsets_.push_back(color_offset);
    return *this;
}

drawer_t& drawer_t::restore_color() {
    color_multiplier = multipliers_.back();
    color_offset = offsets_.back();
    multipliers_.pop_back();
    offsets_.pop_back();
    return *this;
}

drawer_t& drawer_t::multiply_alpha(float alpha) {
    auto a = (uint8_t) ((color_multiplier.a * ((int) (alpha * 255)) * 258u) >> 16u);
    color_multiplier.a = a;
    return *this;
}

drawer_t& drawer_t::multiply_color(argb32_t multiplier) {
    color_multiplier = color_multiplier * multiplier;
    return *this;
}

drawer_t& drawer_t::combine_color(argb32_t multiplier, argb32_t offset) {
    if (offset.argb != 0) {

        using details::clamp_255;
        color_offset = argb32_t(
                clamp_255[((color_offset.r * color_multiplier.r * 258u) >> 16u) + offset.r],
                clamp_255[((color_offset.g * color_multiplier.g * 258u) >> 16u) + offset.g],
                clamp_255[((color_offset.b * color_multiplier.b * 258u) >> 16u) + offset.b],
                offset.a
        );
    }

    if (multiplier.argb != 0xFFFFFFFF) {
        color_multiplier = color_multiplier * multiplier;
    }

    return *this;
}

drawer_t& drawer_t::offset_color(argb32_t offset) {
    argb32_t left_mult = color_multiplier;
    argb32_t left_offset = color_offset;

    using details::clamp_255;
    color_offset = argb32_t(
            clamp_255[((left_offset.r * left_mult.r * 258u) >> 16u) + offset.r],
            clamp_255[((left_offset.g * left_mult.g * 258u) >> 16u) + offset.g],
            clamp_255[((left_offset.b * left_mult.b * 258u) >> 16u) + offset.b],
            offset.a
    );
    return *this;
}

/** STATES **/

drawer_t& drawer_t::save_canvas_rect() {
    canvas_stack_.push_back(canvas_rect_);
    return *this;
}

drawer_t& drawer_t::restore_canvas_rect() {
    canvas_rect_ = canvas_stack_.back();
    canvas_stack_.pop_back();
    return *this;
}

drawer_t& drawer_t::save_projection_matrix() {
    projection_stack_.push_back(batcher.states.get_projection());
    return *this;
}

drawer_t& drawer_t::restore_projection_matrix() {
    batcher.states.set_projection(projection_stack_.back());
    projection_stack_.pop_back();
    return *this;
}

drawer_t& drawer_t::save_texture_coords() {
    tex_coords_stack_.push_back(uv);
    return *this;
}

drawer_t& drawer_t::set_texture_coords(float u0, float v0, float du, float dv) {
    uv.set(u0, v0, du, dv);
    return *this;
}

drawer_t& drawer_t::set_texture_coords(const rect_f& uv_rect) {
    uv = uv_rect;
    return *this;
}

drawer_t& drawer_t::restore_texture_coords() {
    uv = tex_coords_stack_.back();
    tex_coords_stack_.pop_back();
    return *this;
}

drawer_t& drawer_t::save_texture() {
    texture_stack_.push_back(texture_);
    return *this;
}

drawer_t& drawer_t::set_empty_texture() {
    texture_ = default_texture_.get();
    set_texture_coords(0, 0, 1, 1);
    return *this;
}

drawer_t& drawer_t::set_texture(const texture_t* texture) {
    texture_ = texture;
    return *this;
}

drawer_t& drawer_t::set_texture_region(const texture_t* texture, const rect_f& region) {
    texture_ = texture != nullptr ? texture : default_texture_.get();
    uv = region;
    return *this;
}

drawer_t& drawer_t::restore_texture() {
    texture_ = texture_stack_.back();
    texture_stack_.pop_back();
    return *this;
}

drawer_t& drawer_t::save_program() {
    program_stack_.push_back(batcher.states.next().program);
    return *this;
}

drawer_t& drawer_t::restore_program() {
    batcher.states.set_program(program_stack_.back());
    program_stack_.pop_back();
    return *this;
}

drawer_t& drawer_t::save_blend_mode() {
    blend_mode_stack_.push_back(batcher.states.next().blend);
    return *this;
}

drawer_t& drawer_t::restore_blend_mode() {
    batcher.states.set_blend_mode(blend_mode_stack_.back());
    blend_mode_stack_.pop_back();
    return *this;
}

void drawer_t::set_blend_mode(blend_mode blend_mode) {
    batcher.states.set_blend_mode(blend_mode);
}

/////

void drawer_t::draw_indexed_triangles(
        const std::vector<float2>& positions, const std::vector<argb32_t>& colors,
        const std::vector<uint16_t>& indices, const float2& offset, const float2& scale) {

    prepare();
    int verticesTotal = static_cast<int>(positions.size());
    triangles(verticesTotal, indices.size());
    float2 loc_uv;

    for (int i = 0; i < verticesTotal; ++i) {
        float2 local_position = positions[i] * scale + offset;
        write_vertex(
                local_position.x,
                local_position.y,
                loc_uv.x,
                loc_uv.y,
                calc_vertex_color_multiplier(colors[i]),
                vertex_color_offset
        );
    }
    write_indices(indices.data(), indices.size());
}

void drawer_t::line(const float2& start, const float2& end, argb32_t color1, argb32_t color2, float lineWidth1,
                    float lineWidth2) {
    float angle = atan2f(end.y - start.y, end.x - start.x);
    float sn = 0.5f * sinf(angle);
    float cs = 0.5f * cosf(angle);
    float t2sina1 = sn * lineWidth1;
    float t2cosa1 = cs * lineWidth1;
    float t2sina2 = sn * lineWidth2;
    float t2cosa2 = cs * lineWidth2;

    prepare();
    triangles(4, 6);

    auto m1 = calc_vertex_color_multiplier(color1);
    auto m2 = calc_vertex_color_multiplier(color2);
    auto co = vertex_color_offset;

    write_vertex(start.x + t2sina1, start.y - t2cosa1, 0, 0, m1, co);
    write_vertex(end.x + t2sina2, end.y - t2cosa2, 1, 0, m2, co);
    write_vertex(end.x - t2sina2, end.y + t2cosa2, 1, 1, m2, co);
    write_vertex(start.x - t2sina1, start.y + t2cosa1, 0, 1, m1, co);

    write_indices_quad();
}

void drawer_t::line(const float2& start, const float2& end, argb32_t color = 0xFFFFFFFF_argb, float lineWidth = 1.0f) {
    line(start, end, color, color, lineWidth, lineWidth);
}

void drawer_t::line(const float2& start, const float2& end) {
    line(start, end, 0xFFFFFFFF_argb, 1.0f);
}


void drawer_t::line_arc(float x, float y, float r, float angle_from, float angle_to, float line_width, int segments,
                        argb32_t color_inner, argb32_t color_outer) {
    float pi2 = float(math::pi2);
    float da = pi2 / float(segments);
    float a0 = angle_from;
    prepare();
    auto m1 = calc_vertex_color_multiplier(color_inner);
    auto m2 = calc_vertex_color_multiplier(color_outer);
    auto co = vertex_color_offset;
    auto hw = line_width / 2.0f;
    auto r0 = r - hw;
    auto r1 = r + hw;
    while (a0 < angle_to) {

        triangles(4, 6);
        float a1 = fminf(angle_to, a0 + da);
        float cs0 = cosf(a0);
        float sn0 = sinf(a0);
        float cs1 = cosf(a1);
        float sn1 = sinf(a1);

        write_vertex(x + r1 * cs0, y + r1 * sn0, 0.0f, 0.0f, m2, co);
        write_vertex(x + r1 * cs1, y + r1 * sn1, 1, 0, m2, co);
        write_vertex(x + r0 * cs1, y + r0 * sn1, 1, 1, m1, co);
        write_vertex(x + r0 * cs0, y + r0 * sn0, 0, 1, m1, co);

        write_indices_quad();

        a0 += da;
    }
}


}