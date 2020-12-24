#include "drawer.hpp"

#include <ek/util/locator.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/math/matrix_camera.hpp>
#include "draw2d_shader.h"

namespace ek::draw2d {

static Batcher* batcher = nullptr;
drawing_state state{};

void drawing_state::finish() {
    // debug checks
    assert(scissorsStack_.empty());
    assert(matrix_stack_.empty());
    assert(colors_.empty());
    assert(program_stack_.empty());
    assert(texture_stack_.empty());
    assert(tex_coords_stack_.empty());

    scissorsStack_.clear();
    matrix_stack_.clear();
    colors_.clear();
    program_stack_.clear();
    texture_stack_.clear();
    tex_coords_stack_.clear();
}

/** Scissors **/

drawing_state& drawing_state::saveScissors() {
    scissorsStack_.push_back(scissors);
    return *this;
}

void drawing_state::setScissors(const rect_f& rc) {
    scissors = rc;
    checkFlags |= Check_Scissors;
}

void drawing_state::pushClipRect(const rect_f& rc) {
    saveScissors();
    setScissors(clamp_bounds(scissors, rc));
}

drawing_state& drawing_state::popClipRect() {
    scissors = scissorsStack_.back();
    scissorsStack_.pop_back();
    checkFlags |= Check_Scissors;
    return *this;
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

drawing_state& drawing_state::concat(const matrix_2d& r) {
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

drawing_state& drawing_state::scaleAlpha(float alpha) {
    auto a = (uint8_t) ((color.scale.a * ((int) (alpha * 255)) * 258u) >> 16u);
    color.scale.a = a;
    return *this;
}

drawing_state& drawing_state::scaleColor(abgr32_t multiplier) {
    color.scale = color.scale * multiplier;
    return *this;
}

drawing_state& drawing_state::concat(abgr32_t scale, abgr32_t offset) {
    using details::clamp_255;

    if (offset.abgr != 0) {
        color.offset = abgr32_t{clamp_255[color.offset.r + ((offset.r * color.scale.r * 258u) >> 16u)],
                                clamp_255[color.offset.g + ((offset.g * color.scale.g * 258u) >> 16u)],
                                clamp_255[color.offset.b + ((offset.b * color.scale.b * 258u) >> 16u)],
                                clamp_255[color.offset.a + offset.a]};
    }

    if (scale.abgr != 0xFFFFFFFF) {
        color.scale = color.scale * scale;
    }

    return *this;
}

drawing_state& drawing_state::concat(ColorMod32 colorMod) {
    return concat(colorMod.scale, colorMod.offset);
}

drawing_state& drawing_state::offset_color(abgr32_t offset) {
    using details::clamp_255;
    if (offset.abgr != 0) {
        color.offset = abgr32_t{clamp_255[color.offset.r + ((offset.r * color.scale.r * 258u) >> 16u)],
                                clamp_255[color.offset.g + ((offset.g * color.scale.g * 258u) >> 16u)],
                                clamp_255[color.offset.b + ((offset.b * color.scale.b * 258u) >> 16u)],
                                clamp_255[color.offset.a + offset.a]};
    }
    return *this;
}

/** STATES **/

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
    checkFlags |= Check_Texture;
    set_texture_coords(0, 0, 1, 1);
    return *this;
}

drawing_state& drawing_state::set_texture(const graphics::Texture* texture_) {
    texture = texture_;
    checkFlags |= Check_Texture;
    return *this;
}

drawing_state& drawing_state::set_texture_region(const graphics::Texture* texture_, const rect_f& region) {
    texture = texture_ != nullptr ? texture_ : default_texture.get();
    checkFlags |= Check_Texture;
    uv = region;
    return *this;
}

drawing_state& drawing_state::restore_texture() {
    texture = texture_stack_.back();
    checkFlags |= Check_Texture;
    texture_stack_.pop_back();
    return *this;
}

drawing_state& drawing_state::pushProgram(const char* name) {
    program_stack_.push_back(program);
    Res<graphics::Shader> pr{name};
    program = pr.empty() ? default_program.get() : pr.get();
    checkFlags |= Check_Shader;
    return *this;
}

drawing_state& drawing_state::setProgram(const graphics::Shader* program_) {
    program = program_ ? program_ : default_program.get();
    checkFlags |= Check_Shader;
    return *this;
}

drawing_state& drawing_state::saveProgram() {
    program_stack_.push_back(program);
    return *this;
}

drawing_state& drawing_state::restoreProgram() {
    program = program_stack_.back();
    checkFlags |= Check_Shader;
    program_stack_.pop_back();
    return *this;
}

void init() {
    using graphics::Texture;
    using graphics::Shader;
    Res<graphics::Texture>{"empty"}.reset(Texture::createSolid32(4, 4, 0xFFFFFFFFu));

    auto pr = new Shader(draw2d_shader_desc());
    Res<Shader>{"draw2d"}.reset(pr);

    pr = new Shader(draw2d_alpha_shader_desc());
    Res<Shader>{"draw2d_alpha"}.reset(pr);

    pr = new Shader(draw2d_color_shader_desc());
    Res<Shader>{"draw2d_color"}.reset(pr);

    batcher = new Batcher();
}

void beginNewFrame() {
    assert(batcher);
    batcher->beginNewFrame();
}

/*** drawings ***/
void begin(rect_i viewport, const matrix_2d& view, const graphics::Texture* renderTarget) {
    assert(!state.active);
    state.texture = state.default_texture.get();
    state.program = state.default_program.get();
    state.scissors = rect_f{viewport};
    state.checkFlags = 0;
    state.renderTarget = renderTarget;
    state.matrix.set_identity();
    state.color = {};
    state.uv.set(0, 0, 1, 1);
    state.active = true;

    batcher->beginPass();
    batcher->setProgram(state.program->shader, 1);
    batcher->setTexture(state.texture->image);
    batcher->setScissors(viewport);
    batcher->renderTarget = renderTarget;
    if (renderTarget) {
        batcher->mvp = ortho_2d<float>(viewport.x, viewport.bottom(), viewport.width, -viewport.height) * view;
    } else {
        batcher->mvp = ortho_2d<float>(viewport.x, viewport.y, viewport.width, viewport.height) * view;
    }
}

void end() {
    assert(batcher);
    assert(state.active);
    batcher->endPass();
    state.finish();
    state.active = false;
}

void* vertex_memory_ptr_ = nullptr;
uint16_t* index_memory_ptr_ = nullptr;

void write_index(uint16_t index) {
    *(index_memory_ptr_++) = batcher->get_vertex_index(index);
}

void commitStateChanges() {
    if (state.checkFlags & drawing_state::Check_Texture) {
        batcher->setTexture(state.texture->image);
    }
    if (state.checkFlags & drawing_state::Check_Shader) {
        batcher->setProgram(state.program->shader, state.program->numFSImages);
    }
    if (state.checkFlags & drawing_state::Check_Scissors) {
        batcher->setScissors(rect_i{state.scissors});
    }
    state.checkFlags = 0;
}

FrameStats getDrawStats() {
    return batcher ? batcher->stats : FrameStats{};
}

void triangles(int vertex_count, int index_count) {
    if (state.checkFlags != 0) {
        commitStateChanges();
    }
    batcher->alloc_triangles(vertex_count, index_count);
    vertex_memory_ptr_ = batcher->vertex_memory_ptr();
    index_memory_ptr_ = batcher->index_memory_ptr();
}

void quad(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state.color.scale;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t color) {
    triangles(4, 6);

    const auto cm = state.color.scale * color;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t c1, abgr32_t c2, abgr32_t c3, abgr32_t c4) {
    triangles(4, 6);

    const auto cm = state.color.scale;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 0.0f, cm * c1, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm * c2, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm * c3, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm * c4, co);

    write_indices_quad();
}

void quad_rotated(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state.color.scale;
    const auto co = state.color.offset;
    write_vertex(x, y, 0, 1, cm, co);
    write_vertex(x + w, y, 0, 0, cm, co);
    write_vertex(x + w, y + h, 1, 0, cm, co);
    write_vertex(x, y + h, 1, 1, cm, co);

    write_indices_quad();
}

// This function should be moved to the dedicated `indexed draw` mode
void fill_circle(const circle_f& circle, abgr32_t inner_color, abgr32_t outer_color, int segments) {
    triangles(1 + segments, 3 * segments);

    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    const auto co = state.color.offset;
    auto inner_cm = state.color.scale * inner_color;
    auto outer_cm = state.color.scale * outer_color;
    write_vertex(x, y, 0.0f, 0.0f, inner_cm, co);

    const float da = math::pi2 / segments;
    float a = 0.0f;
    while (a < math::pi2) {
        write_vertex(x + r * cosf(a), y + r * sinf(a), 1, 1, outer_cm, co);
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

void write_vertex(float x, float y, float u, float v, abgr32_t cm, abgr32_t co) {
    auto* ptr = static_cast<Vertex2D*>(vertex_memory_ptr_);

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

void write_raw_vertex(const float2& pos, const float2& tex_coord, abgr32_t cm, abgr32_t co) {
    auto* ptr = static_cast<Vertex2D*>(vertex_memory_ptr_);
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

Batcher* getBatcher() {
    return batcher;
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
        const std::vector<float2>& positions, const std::vector<abgr32_t>& colors,
        const std::vector<uint16_t>& indices, const float2& offset, const float2& scale) {

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
                state.color.scale * colors[i],
                state.color.offset
        );
    }
    write_indices(indices.data(), indices.size());
}

void line(const float2& start, const float2& end, abgr32_t color1, abgr32_t color2, float lineWidth1,
          float lineWidth2) {
    float angle = atan2f(end.y - start.y, end.x - start.x);
    float sn = 0.5f * sinf(angle);
    float cs = 0.5f * cosf(angle);
    float t2sina1 = sn * lineWidth1;
    float t2cosa1 = cs * lineWidth1;
    float t2sina2 = sn * lineWidth2;
    float t2cosa2 = cs * lineWidth2;

    triangles(4, 6);

    auto m1 = state.color.scale * color1;
    auto m2 = state.color.scale * color2;
    auto co = state.color.offset;

    write_vertex(start.x + t2sina1, start.y - t2cosa1, 0, 0, m1, co);
    write_vertex(end.x + t2sina2, end.y - t2cosa2, 1, 0, m2, co);
    write_vertex(end.x - t2sina2, end.y + t2cosa2, 1, 1, m2, co);
    write_vertex(start.x - t2sina1, start.y + t2cosa1, 0, 1, m1, co);

    write_indices_quad();
}

void line(const float2& start,
          const float2& end,
          abgr32_t color,
          float lineWidth) {
    line(start, end, color, color, lineWidth, lineWidth);
}

void line_arc(float x, float y, float r,
              float angle_from, float angle_to,
              float line_width, int segments,
              abgr32_t color_inner, abgr32_t color_outer) {
    auto pi2 = static_cast<float>(math::pi2);
    float da = pi2 / float(segments);
    float a0 = angle_from;
    auto m1 = state.color.scale * color_inner;
    auto m2 = state.color.scale * color_outer;
    auto co = state.color.offset;
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

        write_vertex(x + r1 * cs0, y + r1 * sn0, 0, 0, m2, co);
        write_vertex(x + r1 * cs1, y + r1 * sn1, 1, 0, m2, co);
        write_vertex(x + r0 * cs1, y + r0 * sn1, 1, 1, m1, co);
        write_vertex(x + r0 * cs0, y + r0 * sn0, 0, 1, m1, co);

        write_indices_quad();

        a0 += da;
    }
}

void strokeRect(const rect_f& rc, abgr32_t color, float lineWidth) {
    line({rc.x, rc.y}, {rc.right(), rc.y}, color, lineWidth);
    line({rc.right(), rc.y}, {rc.right(), rc.bottom()}, color, lineWidth);
    line({rc.right(), rc.bottom()}, {rc.x, rc.bottom()}, color, lineWidth);
    line({rc.x, rc.bottom()}, {rc.x, rc.y}, color, lineWidth);
}

uint32_t getBatchingUsedMemory() {
    assert(batcher);
    return batcher->getUsedMemory();
}

void endFrame() {
    assert(batcher);
    batcher->completeFrame();
}

}