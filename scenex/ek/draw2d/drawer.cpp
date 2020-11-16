#include "drawer.hpp"

#include <ek/util/locator.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/graphics/gl_def.hpp>
#include <ek/math/matrix_camera.hpp>
#include <ek/graphics/gl_debug.hpp>

namespace ek::draw2d {

Batcher* batcher = nullptr;
drawing_state state{};

void drawing_state::finish() {
    // debug checks
    assert(scissor_stack_.empty());
    assert(matrix_stack_.empty());
    assert(colors_.empty());
    assert(program_stack_.empty());
    assert(texture_stack_.empty());
    assert(blend_mode_stack_.empty());
    assert(mvp_stack_.empty());
    assert(tex_coords_stack_.empty());
    assert(canvas_stack_.empty());

    scissor_stack_.clear();
    matrix_stack_.clear();
    colors_.clear();
    program_stack_.clear();
    texture_stack_.clear();
    blend_mode_stack_.clear();
    mvp_stack_.clear();
    tex_coords_stack_.clear();
    canvas_stack_.clear();
}


/** Scissors **/

void drawing_state::saveScissors() {
    scissor_stack_.push_back(scissors);
}

void drawing_state::setScissors(const rect_f& scissors_) {
    scissors = scissors_;
    check_scissors = true;
}

void drawing_state::push_scissors(const rect_f& scissors_) {
    saveScissors();
    setScissors(clamp_bounds(scissors, scissors_));
}

void drawing_state::pop_scissors() {
    scissors = scissor_stack_.back();
    scissor_stack_.pop_back();
    check_scissors = true;
}

/** Matrix Transform **/

drawing_state& drawing_state::save_matrix() {
    matrix_stack_.push_back(matrix);
    return *this;
}

drawing_state& drawing_state::save_transform() {
    save_matrix();
    save_color();
    return *this;
}

drawing_state& drawing_state::restore_transform() {
    restore_matrix();
    restore_color();
    return *this;
}

drawing_state& drawing_state::translate(float tx, float ty) {
    matrix.translate(tx, ty);
    return *this;
}

drawing_state& drawing_state::translate(const float2& v) {
    matrix.translate(v);
    return *this;
}

drawing_state& drawing_state::scale(float sx, float sy) {
    matrix.scale(sx, sy);
    return *this;
}

drawing_state& drawing_state::scale(const float2& v) {
    matrix.scale(v);
    return *this;
}

drawing_state& drawing_state::rotate(float radians) {
    matrix.rotate(radians);
    return *this;
}

drawing_state& drawing_state::concat_matrix(const matrix_2d& r) {
    matrix = matrix * r;
    return *this;
}

drawing_state& drawing_state::restore_matrix() {
    matrix = matrix_stack_.back();
    matrix_stack_.pop_back();
    return *this;
}

/** Color Transform **/

drawing_state& drawing_state::save_color() {
    colors_.push_back(color);
    return *this;
}

drawing_state& drawing_state::restore_color() {
    color = colors_.back();
    colors_.pop_back();
    return *this;
}

drawing_state& drawing_state::multiply_alpha(float alpha) {
    auto a = (uint8_t) ((color.scale.a * ((int) (alpha * 255)) * 258u) >> 16u);
    color.scale.a = a;
    return *this;
}

drawing_state& drawing_state::multiply_color(argb32_t scale) {
    color.scale = color.scale * scale;
    return *this;
}

drawing_state& drawing_state::combine_color(argb32_t scale, argb32_t offset) {
    using details::clamp_255;

    if (offset.argb != 0) {
        color.offset = argb32_t{clamp_255[color.offset.r + ((offset.r * color.scale.r * 258u) >> 16u)],
                                clamp_255[color.offset.g + ((offset.g * color.scale.g * 258u) >> 16u)],
                                clamp_255[color.offset.b + ((offset.b * color.scale.b * 258u) >> 16u)],
                                clamp_255[color.offset.a + offset.a]};
    }

    if (scale.argb != 0xFFFFFFFF) {
        color.scale = color.scale * scale;
    }

    return *this;
}

drawing_state& drawing_state::offset_color(argb32_t offset) {
    using details::clamp_255;
    if (offset.argb != 0) {
        color.offset = argb32_t{clamp_255[color.offset.r + ((offset.r * color.scale.r * 258u) >> 16u)],
                                clamp_255[color.offset.g + ((offset.g * color.scale.g * 258u) >> 16u)],
                                clamp_255[color.offset.b + ((offset.b * color.scale.b * 258u) >> 16u)],
                                clamp_255[color.offset.a + offset.a]};
    }
    return *this;
}

/** STATES **/

drawing_state& drawing_state::save_canvas_rect() {
    canvas_stack_.push_back(canvas_rect);
    return *this;
}

drawing_state& drawing_state::restore_canvas_rect() {
    canvas_rect = canvas_stack_.back();
    canvas_stack_.pop_back();
    return *this;
}

drawing_state& drawing_state::save_mvp() {
    mvp_stack_.push_back(mvp);
    return *this;
}

drawing_state& drawing_state::set_mvp(const mat4f& m) {
    mvp = m;
    check_mvp = true;
    return *this;
}

drawing_state& drawing_state::restore_mvp() {
    mvp = mvp_stack_.back();
    check_mvp = true;
    mvp_stack_.pop_back();
    return *this;
}

drawing_state& drawing_state::save_texture_coords() {
    tex_coords_stack_.push_back(uv);
    return *this;
}

drawing_state& drawing_state::set_texture_coords(float u0, float v0, float du, float dv) {
    uv.set(u0, v0, du, dv);
    return *this;
}

drawing_state& drawing_state::set_texture_coords(const rect_f& uv_rect) {
    uv = uv_rect;
    return *this;
}

drawing_state& drawing_state::restore_texture_coords() {
    uv = tex_coords_stack_.back();
    tex_coords_stack_.pop_back();
    return *this;
}

drawing_state& drawing_state::save_texture() {
    texture_stack_.push_back(texture);
    return *this;
}

drawing_state& drawing_state::set_empty_texture() {
    texture = default_texture.get();
    check_texture = true;
    set_texture_coords(0, 0, 1, 1);
    return *this;
}

drawing_state& drawing_state::set_texture(const graphics::texture_t* texture_) {
    texture = texture_;
    check_texture = true;
    return *this;
}

drawing_state& drawing_state::set_texture_region(const graphics::texture_t* texture_, const rect_f& region) {
    texture = texture_ != nullptr ? texture_ : default_texture.get();
    check_texture = true;
    uv = region;
    return *this;
}

drawing_state& drawing_state::restore_texture() {
    texture = texture_stack_.back();
    check_texture = true;
    texture_stack_.pop_back();
    return *this;
}

drawing_state& drawing_state::set_program(const graphics::program_t* program_) {
    program = program_ ? program_ : default_program.get();
    check_program = true;
    return *this;
}

drawing_state& drawing_state::save_program() {
    program_stack_.push_back(program);
    return *this;
}

drawing_state& drawing_state::restore_program() {
    program = program_stack_.back();
    check_program = true;
    program_stack_.pop_back();
    return *this;
}

drawing_state& drawing_state::save_blend_mode() {
    blend_mode_stack_.push_back(blending);
    return *this;
}

drawing_state& drawing_state::restore_blend_mode() {
    blending = blend_mode_stack_.back();
    blend_mode_stack_.pop_back();
    check_blending = true;
    return *this;
}

void drawing_state::set_blend_mode(graphics::blend_mode blending_) {
    blending = blending_;
    check_blending = true;
}


/*** drawings ***/
void begin(int x, int y, int width, int height) {
    if (!batcher) {
        batcher = new Batcher;
    }
    batcher->stats = {};
    batcher->begin();

    state.texture = state.default_texture.get();
    state.program = state.default_program.get();
    state.blending = graphics::blend_mode::premultiplied;
    state.mvp = ortho_2d<float>(x, y, width, height);
    state.scissors.set(x, y, width, height);
    state.check_program = false;
    state.check_scissors = false;
    state.check_blending = false;
    state.check_mvp = false;
    state.check_texture = false;

    auto& batcher_states = batcher->states;
    batcher_states.clear();
    batcher_states.set_program(state.program);
    batcher_states.set_mvp(state.mvp);
    batcher_states.set_blend_mode(state.blending);
    batcher_states.set_texture(state.texture);
    batcher_states.set_scissors(state.scissors);
}

void end() {
    batcher->states.apply();
    batcher->flush();

    GL_CHECK(glBindBuffer(GL_ARRAY_BUFFER, (GLuint) 0u));
    GL_CHECK(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0u));
    GL_CHECK(glBindTexture(GL_TEXTURE_2D, 0u));
    GL_CHECK(glUseProgram(0u));

    state.finish();
}

void* vertex_memory_ptr_ = nullptr;
uint16_t* index_memory_ptr_ = nullptr;

void write_index(uint16_t index) {
    *(index_memory_ptr_++) = batcher->get_vertex_index(index);
}

void commit_state() {
    if (state.check_blending) {
        batcher->states.set_blend_mode(state.blending);
        state.check_blending = false;
    }
    if (state.check_texture) {
        batcher->states.set_texture(state.texture);
        state.check_texture = false;
    }
    if (state.check_program) {
        batcher->states.set_program(state.program);
        state.check_program = false;
    }
    if (state.check_mvp) {
        batcher->states.set_mvp(state.mvp);
        state.check_mvp = false;
    }
    if (state.check_scissors) {
        batcher->states.set_scissors(state.scissors);
        state.check_scissors = false;
    }
}

uint32_t get_stat_draw_calls() {
    return batcher ? batcher->stats.draw_calls : 0;
}

uint32_t get_stat_triangles() {
    return batcher ? batcher->stats.triangles : 0;
}

void invalidate_force() {
    commit_state();
    batcher->invalidate_force();
}

void draw_mesh(const graphics::buffer_t& vb, const graphics::buffer_t& ib, int32_t indices_count) {
    batcher->draw_mesh(vb, ib, indices_count);
}

void flush_batcher() {
    batcher->flush();
}

void prepare() {
    state.vertex_color_multiplier = state.color.scale.premultiplied_abgr(state.color.offset.a);
    // for offset: delete alpha, flip R vs B channels
    state.vertex_color_offset = state.color.offset.bgr();
}

void triangles(int vertex_count, int index_count) {
    commit_state();
    batcher->alloc_triangles(vertex_count, index_count);
    vertex_memory_ptr_ = batcher->vertex_memory_ptr();
    index_memory_ptr_ = batcher->index_memory_ptr();
}

void quad(float x, float y, float w, float h) {
    prepare();

    triangles(4, 6);

    const auto cm = state.vertex_color_multiplier;
    const auto co = state.vertex_color_offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, argb32_t color) {
    prepare();

    triangles(4, 6);

    const auto cm = state.calc_vertex_color_multiplier(color);
    const auto co = state.vertex_color_offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, argb32_t c1, argb32_t c2, argb32_t c3, argb32_t c4) {
    prepare();

    triangles(4, 6);

    const auto co = state.vertex_color_offset;
    write_vertex(x, y, 0, 0.0f, state.calc_vertex_color_multiplier(c1), co);
    write_vertex(x + w, y, 1.0f, 0.0f, state.calc_vertex_color_multiplier(c2), co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, state.calc_vertex_color_multiplier(c3), co);
    write_vertex(x, y + h, 0.0f, 1.0f, state.calc_vertex_color_multiplier(c4), co);

    write_indices_quad();
}

void quad_rotated(float x, float y, float w, float h) {
    prepare();

    triangles(4, 6);

    const auto cm = state.vertex_color_multiplier;
    const auto co = state.vertex_color_offset;
    write_vertex(x, y, 0, 1, cm, co);
    write_vertex(x + w, y, 0, 0, cm, co);
    write_vertex(x + w, y + h, 1, 0, cm, co);
    write_vertex(x, y + h, 1, 1, cm, co);

    write_indices_quad();
}

void fill_circle(const circle_f& circle, argb32_t inner_color, argb32_t outer_color, int segments) {
    prepare();
    triangles(1 + segments, 3 * segments);

    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    auto inner_cm = state.calc_vertex_color_multiplier(inner_color);
    auto outer_cm = state.calc_vertex_color_multiplier(outer_color);
    write_vertex(x, y, 0.0f, 0.0f, inner_cm, state.vertex_color_offset);

    const float da = math::pi2 / segments;
    float a = 0.0f;
    while (a < math::pi2) {
        write_vertex(x + r * cosf(a), y + r * sinf(a), 1, 1, outer_cm, state.vertex_color_offset);
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

void write_vertex(float x, float y, float u, float v, premultiplied_abgr32_t cm, abgr32_t co) {
    auto* ptr = static_cast<graphics::vertex_2d*>(vertex_memory_ptr_);

    // could be cached before draw2d
    const auto& m = state.matrix;
    const auto& uv = state.uv;

    ptr->position.x = x * m.a + y * m.c + m.tx;
    ptr->position.y = x * m.b + y * m.d + m.ty;
    ptr->uv.x = uv.x + u * uv.width;
    ptr->uv.y = uv.y + v * uv.height;
    ptr->cm = cm;
    ptr->co = co;
    ++ptr;

    vertex_memory_ptr_ = (uint8_t*) ptr;
}

void write_raw_vertex(const float2& pos, const float2& tex_coord, premultiplied_abgr32_t cm, abgr32_t co) {
    auto* ptr = static_cast<graphics::vertex_2d*>(vertex_memory_ptr_);
    ptr->position = pos;
    ptr->uv = tex_coord;
    ptr->cm = cm;
    ptr->co = co;
    ++ptr;
    vertex_memory_ptr_ = (uint8_t*) ptr;
}

void write_indices_quad(const uint16_t i0,
                        const uint16_t i1,
                        const uint16_t i2,
                        const uint16_t i3,
                        const uint16_t base_vertex) {
    const uint16_t index = batcher->get_vertex_index(base_vertex);
    *(index_memory_ptr_++) = index + i0;
    *(index_memory_ptr_++) = index + i1;
    *(index_memory_ptr_++) = index + i2;
    *(index_memory_ptr_++) = index + i2;
    *(index_memory_ptr_++) = index + i3;
    *(index_memory_ptr_++) = index + i0;
}

void write_indices(const uint16_t* source,
                   uint16_t count,
                   uint16_t base_vertex) {
    const uint16_t index = batcher->get_vertex_index(base_vertex);
    for (int i = 0; i < count; ++i) {
        *index_memory_ptr_ = (*source) + index;
        ++index_memory_ptr_;
        ++source;
    }
}

/////

void draw_indexed_triangles(
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
                state.calc_vertex_color_multiplier(colors[i]),
                state.vertex_color_offset
        );
    }
    write_indices(indices.data(), indices.size());
}

void line(const float2& start, const float2& end, argb32_t color1, argb32_t color2, float lineWidth1,
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

    auto m1 = state.calc_vertex_color_multiplier(color1);
    auto m2 = state.calc_vertex_color_multiplier(color2);
    auto co = state.vertex_color_offset;

    write_vertex(start.x + t2sina1, start.y - t2cosa1, 0, 0, m1, co);
    write_vertex(end.x + t2sina2, end.y - t2cosa2, 1, 0, m2, co);
    write_vertex(end.x - t2sina2, end.y + t2cosa2, 1, 1, m2, co);
    write_vertex(start.x - t2sina1, start.y + t2cosa1, 0, 1, m1, co);

    write_indices_quad();
}

void line(const float2& start,
          const float2& end,
          argb32_t color = 0xFFFFFFFF_argb,
          float lineWidth = 1.0f) {
    line(start, end, color, color, lineWidth, lineWidth);
}

void line(const float2& start, const float2& end) {
    line(start, end, 0xFFFFFFFF_argb, 1.0f);
}


void line_arc(float x, float y, float r,
              float angle_from, float angle_to,
              float line_width, int segments,
              argb32_t color_inner, argb32_t color_outer) {
    auto pi2 = static_cast<float>(math::pi2);
    float da = pi2 / float(segments);
    float a0 = angle_from;
    prepare();
    auto m1 = state.calc_vertex_color_multiplier(color_inner);
    auto m2 = state.calc_vertex_color_multiplier(color_outer);
    auto co = state.vertex_color_offset;
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

void strokeRect(const rect_f& rc, argb32_t color, float lineWidth) {
    line({rc.x, rc.y}, {rc.right(), rc.y}, color, lineWidth);
    line({rc.right(), rc.y}, {rc.right(), rc.bottom()}, color, lineWidth);
    line({rc.right(), rc.bottom()}, {rc.x, rc.bottom()}, color, lineWidth);
    line({rc.x, rc.bottom()}, {rc.x, rc.y}, color, lineWidth);
}

uint32_t getBatchingUsedMemory() {
    return batcher ? batcher->getUsedMemory() : 0;
}

}