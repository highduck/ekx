#include "drawer.hpp"

#include <ek/util/Platform.hpp>
#include <ek/util/StaticStorage.hpp>
#include <ek/math/MatrixCamera.hpp>
#include "draw2d_shader.h"
#include <ek/log.h>
#include <ek/assert.h>

/// region Buffers Chain

void ek_canvas_buffers_init(ek_canvas_buffers* buffers, sg_buffer_type type, uint32_t elements_max_count,
                            uint32_t element_max_size) {
    EK_ASSERT(elements_max_count > 0x400);
    buffers->type = type;
    const uint32_t c = 0x10 * element_max_size;
    buffers->cap[0] = c;
    buffers->cap[1] = c << 4;
    buffers->cap[2] = c << 6;
    buffers->cap[3] = elements_max_count * element_max_size;
}

int ek_canvas_buffers_get_bucket(ek_canvas_buffers* buffers, uint32_t required_size) {
    uint32_t* cap = buffers->cap;
    if (required_size < cap[0]) {
        return 0;
    } else if (required_size < cap[1]) {
        return 1;
    } else if (required_size < cap[2]) {
        return 2;
    }
    return 3;
}

sg_buffer ek_canvas_buffers_get(ek_canvas_buffers* buffers, uint32_t required_size) {

    const int bucket = ek_canvas_buffers_get_bucket(buffers, required_size);
    sg_buffer* line = buffers->lines[bucket];
    uint16_t index = buffers->pos[bucket];
    if (index == EK_CANVAS_BUFFERS_MAX_COUNT) {
        return (sg_buffer) {0};
    }
    sg_buffer buf = line[index];
    if (buf.id == 0) {
        sg_buffer_desc desc = (sg_buffer_desc) {
                .size = buffers->cap[bucket],
                .type = buffers->type,
                .usage = SG_USAGE_STREAM,
                .label = buffers->type == SG_BUFFERTYPE_VERTEXBUFFER ? "canvas-vbs" : "canvas-ibs"
        };
        buf = sg_make_buffer(&desc);
        line[index] = buf;
    }
    if (buf.id) {
        ++buffers->pos[bucket];
    }
    return buf;
}

void ek_canvas_buffers_rewind(ek_canvas_buffers* buffers) {
    uint16_t* positions = buffers->pos;
    for (int i = 0; i < 4; ++i) {
        positions[i] = 0;
    }
}

void ek_canvas_buffers_destroy(ek_canvas_buffers* buffers) {
    for (int bucket = 0; bucket < 4; ++bucket) {
        sg_buffer* line = buffers->lines[bucket];
        int count = buffers->pos[bucket];
        for (int i = 0; i < count; ++i) {
            sg_destroy_buffer(line[i]);
        }
    }
}

/// endregion

sg_layout_desc ek_vertex2d_layout_desc(void) {
    using ek::draw2d::Vertex2D;
    sg_layout_desc layout = {};
    layout.buffers[0].stride = sizeof(Vertex2D);
    layout.attrs[0].offset = offsetof(Vertex2D, position);
    layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[1].offset = offsetof(Vertex2D, uv);
    layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[2].offset = offsetof(Vertex2D, cm);
    layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
    layout.attrs[3].offset = offsetof(Vertex2D, co);
    layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    return layout;
}

ek_canvas_context ek_canvas_;

static void ek_canvas__reset_stream(void) {
    // just verify that we alloc before write
    ek_canvas_.vertex_it = NULL;
    ek_canvas_.index_it = NULL;

    ek_canvas_.vertex_end = ek_canvas_.vertex;
    ek_canvas_.index_end = ek_canvas_.index;

    ek_canvas_.vertex_num = 0;
    ek_canvas_.index_num = 0;
}

namespace ek::draw2d {

StaticStorage<Context> context;
Context& state = context.ref();

void Context::setNextScissors(Rect2i rc) {
    const ek_canvas_scissors r = {
            (int16_t) rc.x,
            (int16_t) rc.y,
            (int16_t) rc.width,
            (int16_t) rc.height
    };
    if (r.packed != ek_canvas_.curr.scissors.packed) {
        ek_canvas_.state |= EK_CANVAS_STATE_CHANGED;
    }
    ek_canvas_.next.scissors = r;
}

void Context::setNextBlending(ek_canvas_blend blend) {
    if (ek_canvas_.curr.blend != blend) {
        ek_canvas_.state |= EK_CANVAS_STATE_CHANGED;
    }
    ek_canvas_.next.blend = blend;
}

void Context::setNextImage(sg_image image_) {
    if (ek_canvas_.curr.image.id != image_.id) {
        ek_canvas_.state |= EK_CANVAS_STATE_CHANGED;
    }
    ek_canvas_.next.image = image_;
}

void Context::setNextShader(ek_shader shader_) {
    if (ek_canvas_.curr.shader.shader.id != shader_.shader.id) {
        ek_canvas_.state |= EK_CANVAS_STATE_CHANGED;
    }
    ek_canvas_.next.shader = shader_;
}

void Context::applyNextState() {
    if (ek_canvas_.state & EK_CANVAS_STATE_CHANGED) {
        ek_canvas_.curr = ek_canvas_.next;
        ek_canvas_.state ^= EK_CANVAS_STATE_CHANGED;
    }
}

sg_pipeline create_pipeline_for_cache(sg_shader shader, bool useRenderTarget, bool depthStencil) {
    sg_pipeline_desc pip_desc{};
    pip_desc.layout = ek_vertex2d_layout_desc();
    pip_desc.shader = shader;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.colors[0].write_mask = SG_COLORMASK_RGB;
    pip_desc.colors[0].blend.enabled = true;
    pip_desc.colors[0].blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
    pip_desc.colors[0].blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_desc.sample_count = 1;
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    if (useRenderTarget) {
        //pip_desc.colors[0].pixel_format = SG_PIXELFORMAT_RGBA8;
        if (depthStencil) {
            pip_desc.depth.pixel_format = SG_PIXELFORMAT_DEPTH_STENCIL;
//            pip_desc.depth.write_enabled = false;
//            pip_desc.depth.compare = SG_COMPAREFUNC_LESS_EQUAL;
        } else {
            pip_desc.depth.pixel_format = SG_PIXELFORMAT_NONE;
        }
    }
    pip_desc.label = "draw2d-pipeline";
    return sg_make_pipeline(pip_desc);
}

sg_pipeline get_pipeline(sg_shader shader, bool useRenderTarget, bool depthStencilPass) {
    uint32_t mode = 0;
    if (useRenderTarget) {
        mode |= 0x1;
    }
    if (depthStencilPass) {
        mode |= 0x2;
    }
    for (int i = 0; i < ek_canvas_.pipelines_num; ++i) {
        ek_canvas_pipeline_item item = ek_canvas_.pipelines[i];
        if (item.shader.id == shader.id && item.mode == mode) {
            return item.pipeline;
        }
    }
    EK_ASSERT(ek_canvas_.pipelines_num != EK_CANVAS_PIPELINE_CACHE_MAX_COUNT);
    ek_canvas_pipeline_item item;
    item.mode = mode;
    item.shader = shader;
    item.pipeline = create_pipeline_for_cache(shader, useRenderTarget, depthStencilPass);

    ek_canvas_.pipelines[ek_canvas_.pipelines_num++] = item;
    log_debug("pipelines: %d", ek_canvas_.pipelines_num);

    return item.pipeline;
}

float triangle_area(const ek_vertex2d* vertices, const uint16_t* indices, int count) {
    float sum = 0.0f;
    for (int i = 0; i < count;) {
        const ek_vertex2d a = vertices[indices[i++]];
        const ek_vertex2d b = vertices[indices[i++]];
        const ek_vertex2d c = vertices[indices[i++]];
        sum += a.x * b.y + b.x * c.y + c.x * a.y - a.x * c.y - b.x * a.y - c.x * b.y;
    }
    return sum / 2.0f;
}

void Context::drawUserBatch(sg_pipeline pip, uint32_t images_count) {
    int index_num = (int) ek_canvas_.index_num;
    if (index_num == 0) {
        return;
    }

    const uint32_t vertexDataSize = (uint32_t) (ek_canvas_.vertex_num * sizeof(ek_vertex2d));
    const uint32_t indexDataSize = (uint32_t) index_num << 1u;
    const sg_buffer vb = ek_canvas_buffers_get(&ek_canvas_.vbs, vertexDataSize);
    const sg_buffer ib = ek_canvas_buffers_get(&ek_canvas_.ibs, indexDataSize);
    EK_ASSERT(vb.id != 0);
    EK_ASSERT(ib.id != 0);
    sg_update_buffer(vb, (sg_range) {ek_canvas_.vertex, vertexDataSize});
    sg_update_buffer(ib, (sg_range) {ek_canvas_.index, indexDataSize});

    // reset current pipeline
    ek_canvas_.pipeline = pip;

    ek_canvas_.bind.vertex_buffers[0] = vb;
    ek_canvas_.bind.index_buffer = ib;
    ek_canvas_.bind.fs_images[0].id = images_count == 1 ? ek_canvas_.curr.image.id : SG_INVALID_ID;
    sg_apply_bindings(ek_canvas_.bind);

    {
        const ek_canvas_scissors rc = ek_canvas_.curr.scissors;
        //sg_apply_viewportf(rc.x, rc.y, rc.width, rc.height, true);
        sg_apply_scissor_rect(rc.x, rc.y, rc.w, rc.h, true);
    }

    sg_draw(0, index_num, 1);

#ifndef NDEBUG
    ek_canvas_.stats.fill_area += triangle_area(ek_canvas_.vertex, ek_canvas_.index, index_num);
#endif
    ek_canvas_.stats.triangles += index_num / 3;
    ++ek_canvas_.stats.draw_calls;

    ek_canvas__reset_stream();
}

void Context::drawBatch() {
    int index_num = (int) ek_canvas_.index_num;
    if (index_num == 0) {
        return;
    }

    const uint32_t vertexDataSize = (uint32_t) (ek_canvas_.vertex_num * sizeof(ek_vertex2d));
    const uint32_t indexDataSize = index_num << 1u;
    const sg_buffer vb = ek_canvas_buffers_get(&ek_canvas_.vbs, vertexDataSize);
    const sg_buffer ib = ek_canvas_buffers_get(&ek_canvas_.ibs, indexDataSize);
    EK_ASSERT(vb.id != 0);
    EK_ASSERT(ib.id != 0);
    sg_update_buffer(vb, {ek_canvas_.vertex, vertexDataSize});
    sg_update_buffer(ib, {ek_canvas_.index, indexDataSize});

    const sg_image fb_color = ek_canvas_.render_target_color.id ? ek_canvas_.render_target_color
                                                                : ek_canvas_.framebuffer_color;
    const sg_image fb_depth = ek_canvas_.render_target_depth.id ? ek_canvas_.render_target_depth
                                                                : ek_canvas_.framebuffer_depth;

    auto pip = get_pipeline(ek_canvas_.curr.shader.shader, fb_color.id != 0, fb_depth.id != 0);
    if (pip.id != ek_canvas_.pipeline.id) {
        ek_canvas_.pipeline = pip;
        sg_apply_pipeline(pip);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE(mvp));
    }
    ek_canvas_.bind.vertex_buffers[0] = vb;
    ek_canvas_.bind.index_buffer = ib;
    ek_canvas_.bind.fs_images[0].id = ek_canvas_.curr.shader.images_num == 1 ? ek_canvas_.curr.image.id : SG_INVALID_ID;
    sg_apply_bindings(ek_canvas_.bind);

    {
        const auto rc = ek_canvas_.curr.scissors;
        //sg_apply_viewportf(rc.x, rc.y, rc.width, rc.height, true);
        sg_apply_scissor_rect(rc.x, rc.y, rc.w, rc.h, true);
    }

    sg_draw(0, index_num, 1);

#ifndef NDEBUG
    // TODO: add mvp matrix and viewport to get real pixels
    ek_canvas_.stats.fill_area += triangle_area(ek_canvas_.vertex, ek_canvas_.index, index_num);
#endif
    ek_canvas_.stats.triangles += index_num / 3;
    ++ek_canvas_.stats.draw_calls;

    ek_canvas__reset_stream();
}

void Context::allocTriangles(uint32_t vertex_count, uint32_t index_count) {
    if (ek_canvas_.state & EK_CANVAS_CHECK_MASK) {
        if (ek_canvas_.state & EK_CANVAS_CHECK_IMAGE) {
            setNextImage(image[0]);
            ek_canvas_.state ^= EK_CANVAS_CHECK_IMAGE;
        }
        if (ek_canvas_.state & EK_CANVAS_CHECK_SHADER) {
            setNextShader(shader[0]);
            ek_canvas_.state ^= EK_CANVAS_CHECK_SHADER;
        }
        if (ek_canvas_.state & EK_CANVAS_CHECK_SCISSORS) {
            setNextScissors(Rect2i{scissors[0]});
            ek_canvas_.state ^= EK_CANVAS_CHECK_SCISSORS;
        }
    }

    if ((ek_canvas_.state & EK_CANVAS_STATE_CHANGED) ||
        (ek_canvas_.vertex_num + vertex_count) > EK_CANVAS_VERTEX_LIMIT) {
        drawBatch();
        applyNextState();
    }

    ek_canvas_.vertex_it = ek_canvas_.vertex_end;
    ek_canvas_.index_it = ek_canvas_.index_end;

    ek_canvas_.vertex_end += vertex_count;
    ek_canvas_.index_end += index_count;

    ek_canvas_.vertex_base = ek_canvas_.vertex_num;
    ek_canvas_.vertex_num += vertex_count;
    ek_canvas_.index_num += index_count;
}

void Context::check_and_reset_stack() {
    // debug checks
    EK_ASSERT(image_top == 0);
    EK_ASSERT(shader_top == 0);
    EK_ASSERT(scissors_top == 0);
    EK_ASSERT(matrix_top == 0);
    EK_ASSERT(color_top == 0);
    EK_ASSERT(uv_top == 0);

    image_top = 0;
    shader_top = 0;
    scissors_top = 0;
    matrix_top = 0;
    color_top = 0;
    uv_top = 0;
}

/** Scissors **/

Context& Context::saveScissors() {
    ++scissors_top;
    EK_ASSERT(scissors_top != EK_CANVAS_STACK_MAX_DEPTH);
    scissors[scissors_top] = scissors[0];
    return *this;
}

void Context::setScissors(const Rect2f& rc) {
    scissors[0] = rc;
    ek_canvas_.state |= EK_CANVAS_CHECK_SCISSORS;
}

void Context::pushClipRect(const Rect2f& rc) {
    saveScissors();
    setScissors(clamp_bounds(scissors[0], rc));
}

Context& Context::popClipRect() {
    EK_ASSERT(scissors_top != 0);
    scissors[0] = scissors[scissors_top--];
    ek_canvas_.state |= EK_CANVAS_CHECK_SCISSORS;
    return *this;
}

/** Matrix Transform **/

Context& Context::save_matrix() {
    ++matrix_top;
    EK_ASSERT(matrix_top != EK_CANVAS_STACK_MAX_DEPTH);
    matrix[matrix_top] = matrix[0];
    return *this;
}

Context& Context::save_transform() {
    save_matrix();
    save_color();
    return *this;
}

Context& Context::restore_transform() {
    restore_matrix();
    restore_color();
    return *this;
}

Context& Context::translate(float tx, float ty) {
    matrix[0].translate(tx, ty);
    return *this;
}

Context& Context::translate(const Vec2f& v) {
    matrix[0].translate(v);
    return *this;
}

Context& Context::scale(float sx, float sy) {
    matrix[0].scale(sx, sy);
    return *this;
}

Context& Context::scale(const Vec2f& v) {
    matrix[0].scale(v);
    return *this;
}

Context& Context::rotate(float radians) {
    matrix[0].rotate(radians);
    return *this;
}

Context& Context::concat(const Matrix3x2f& r) {
    matrix[0] = matrix[0] * r;
    return *this;
}

Context& Context::restore_matrix() {
    EK_ASSERT(matrix_top != 0);
    matrix[0] = matrix[matrix_top--];
    return *this;
}

/** Color Transform **/

Context& Context::save_color() {
    ++color_top;
    EK_ASSERT(color_top != EK_CANVAS_STACK_MAX_DEPTH);
    color[color_top] = color[0];
    return *this;
}

Context& Context::restore_color() {
    EK_ASSERT(color_top != 0);
    color[0] = color[color_top--];
    return *this;
}

Context& Context::scaleAlpha(float alpha) {
    auto a = (uint8_t) ((color[0].scale.a * ((int) (alpha * 255)) * 258u) >> 16u);
    color[0].scale.a = a;
    return *this;
}

Context& Context::scaleColor(abgr32_t multiplier) {
    color[0].scale = color[0].scale * multiplier;
    return *this;
}

Context& Context::concat(abgr32_t scale, abgr32_t offset) {
    if (offset.abgr != 0) {
        ColorMod32 color_ = color[0];
        color[0].offset = abgr32_t{sat_add_u8(color_.offset.r, ((offset.r * color_.scale.r * 258u) >> 16u)),
                                   sat_add_u8(color_.offset.g, ((offset.g * color_.scale.g * 258u) >> 16u)),
                                   sat_add_u8(color_.offset.b, ((offset.b * color_.scale.b * 258u) >> 16u)),
                                   sat_add_u8(color_.offset.a, offset.a)};
    }

    if (scale.abgr != 0xFFFFFFFF) {
        color[0].scale = color[0].scale * scale;
    }

    return *this;
}

Context& Context::concat(ColorMod32 colorMod) {
    return concat(colorMod.scale, colorMod.offset);
}

Context& Context::offset_color(abgr32_t offset) {
    if (offset.abgr != 0) {
        ColorMod32 color_ = color[0];
        color[0].offset = abgr32_t{sat_add_u8(color_.offset.r, ((offset.r * color_.scale.r * 258u) >> 16u)),
                                   sat_add_u8(color_.offset.g, ((offset.g * color_.scale.g * 258u) >> 16u)),
                                   sat_add_u8(color_.offset.b, ((offset.b * color_.scale.b * 258u) >> 16u)),
                                   sat_add_u8(color_.offset.a, offset.a)};
    }
    return *this;
}

/** STATES **/

Context& Context::save_image_rect() {
    ++uv_top;
    EK_ASSERT(uv_top != EK_CANVAS_STACK_MAX_DEPTH);
    uv[uv_top] = uv[0];
    return *this;
}

Context& Context::set_image_rect(float u0, float v0, float du, float dv) {
    uv[0].set(u0, v0, du, dv);
    return *this;
}

Context& Context::set_image_rect(const Rect2f& uv_rect) {
    uv[0] = uv_rect;
    return *this;
}

Context& Context::restore_image_rect() {
    EK_ASSERT(uv_top != 0);
    uv[0] = uv[uv_top--];
    return *this;
}

Context& Context::save_image() {
    ++image_top;
    EK_ASSERT(image_top != EK_CANVAS_STACK_MAX_DEPTH);
    image[image_top] = image[0];

    return *this;
}

Context& Context::set_empty_image() {
    image[0] = ek_canvas_.image_empty;
    ek_canvas_.state |= EK_CANVAS_CHECK_IMAGE;
    set_image_rect(0, 0, 1, 1);
    return *this;
}

Context& Context::set_image(sg_image image_) {
    image[0] = image_;
    ek_canvas_.state |= EK_CANVAS_CHECK_IMAGE;
    return *this;
}

Context& Context::set_image_region(sg_image image_, const Rect2f& region) {
    image[0] = image_.id ? image_ : ek_canvas_.image_empty;
    ek_canvas_.state |= EK_CANVAS_CHECK_IMAGE;
    uv[0] = region;
    return *this;
}

Context& Context::restore_image() {
    EK_ASSERT(image_top != 0);
    image[0] = image[image_top--];
    ek_canvas_.state |= EK_CANVAS_CHECK_IMAGE;
    return *this;
}

Context& Context::pushProgram(const ek_shader program_) {
    saveProgram();
    setProgram(program_);
    return *this;
}

Context& Context::setProgram(const ek_shader program_) {
    shader[0] = program_.shader.id ? program_ : ek_canvas_.shader_default;
    ek_canvas_.state |= EK_CANVAS_CHECK_SHADER;
    return *this;
}

Context& Context::saveProgram() {
    ++shader_top;
    EK_ASSERT(shader_top != EK_CANVAS_STACK_MAX_DEPTH);
    shader[shader_top] = shader[0];
    return *this;
}

Context& Context::restoreProgram() {
    EK_ASSERT(shader_top != 0);
    shader[0] = shader[shader_top--];
    ek_canvas_.state |= EK_CANVAS_CHECK_SHADER;
    return *this;
}

/*** drawings ***/
void begin(Rect2f viewport, const Matrix3x2f& view, const sg_image renderTarget, const sg_image depthStencilTarget) {
    EK_ASSERT(!(ek_canvas_.state & EK_CANVAS_PASS_ACTIVE));
    // reset all bits and set Active mode / dirty state flag
    ek_canvas_.state = EK_CANVAS_PASS_ACTIVE | EK_CANVAS_STATE_CHANGED;

    state.image[0] = ek_canvas_.image_empty;
    state.shader[0] = ek_canvas_.shader_default;
    state.scissors[0] = viewport;
    state.matrix[0].set_identity();
    state.color[0] = {};
    state.uv[0].set(0, 0, 1, 1);

    ek_canvas_.curr = {};
    ek_canvas_.next.shader = state.shader[0];
    ek_canvas_.next.image = state.image[0];
    ek_canvas_.next.scissors = {
            (int16_t) viewport.x,
            (int16_t) viewport.y,
            (int16_t) viewport.width,
            (int16_t) viewport.height,
    };
    ek_canvas_.pipeline.id = SG_INVALID_ID;

    ek_canvas_.render_target_color = renderTarget;
    ek_canvas_.render_target_depth = depthStencilTarget;

    Matrix4f proj;
#if EK_IOS || EK_MACOS
    proj = ortho_2d<float>(viewport.x, viewport.y, viewport.width, viewport.height);
#else
    if (ek_canvas_.render_target_color.id) {
        proj = ortho_2d<float>(viewport.x, viewport.bottom(), viewport.width, -viewport.height);
    } else {
        proj = ortho_2d<float>(viewport.x, viewport.y, viewport.width, viewport.height);
    }
#endif

    state.mvp = proj * view;
}

void end() {
    EK_ASSERT(ek_canvas_.state & EK_CANVAS_PASS_ACTIVE);
    state.drawBatch();
    state.check_and_reset_stack();
    ek_canvas_.state ^= EK_CANVAS_PASS_ACTIVE;
}

void write_index(uint16_t index) {
    *(ek_canvas_.index_it++) = ek_canvas_.vertex_base + index;
}

void triangles(int vertex_count, int index_count) {
    state.allocTriangles(vertex_count, index_count);
}

void quad(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state.color[0].scale;
    const auto co = state.color[0].offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t color) {
    triangles(4, 6);

    const auto cm = state.color[0].scale * color;
    const auto co = state.color[0].offset;
    write_vertex(x, y, 0, 0.0f, cm, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    write_indices_quad();
}

void quad(float x, float y, float w, float h, abgr32_t c1, abgr32_t c2, abgr32_t c3, abgr32_t c4) {
    triangles(4, 6);

    const auto cm = state.color[0].scale;
    const auto co = state.color[0].offset;
    write_vertex(x, y, 0, 0.0f, cm * c1, co);
    write_vertex(x + w, y, 1.0f, 0.0f, cm * c2, co);
    write_vertex(x + w, y + h, 1.0f, 1.0f, cm * c3, co);
    write_vertex(x, y + h, 0.0f, 1.0f, cm * c4, co);

    write_indices_quad();
}

void quad_rotated(float x, float y, float w, float h) {
    triangles(4, 6);

    const auto cm = state.color[0].scale;
    const auto co = state.color[0].offset;
    write_vertex(x, y, 0, 1, cm, co);
    write_vertex(x + w, y, 0, 0, cm, co);
    write_vertex(x + w, y + h, 1, 0, cm, co);
    write_vertex(x, y + h, 1, 1, cm, co);

    write_indices_quad();
}

// This function should be moved to the dedicated `indexed draw` mode
void fill_circle(const CircleF& circle, abgr32_t inner_color, abgr32_t outer_color, int segments) {
    triangles(1 + segments, 3 * segments);

    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    const auto co = state.color[0].offset;
    auto inner_cm = state.color[0].scale * inner_color;
    auto outer_cm = state.color[0].scale * outer_color;
    write_vertex(x, y, 0.0f, 0.0f, inner_cm, co);

    const float da = Math::fPI2 / (float) segments;
    float a = 0.0f;
    while (a < Math::pi2) {
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
    // could be cached before draw2d
    const auto m = state.matrix[0];
    const auto uv = state.uv[0];

    auto* ptr = ek_canvas_.vertex_it++;
    ptr->x = x * m.a + y * m.c + m.tx;
    ptr->y = x * m.b + y * m.d + m.ty;
    ptr->u = uv.x + u * uv.width;
    ptr->v = uv.y + v * uv.height;
    ptr->cm = cm.abgr;
    ptr->co = co.abgr;
}

void write_raw_vertex(const Vec2f& pos, const Vec2f& tex_coord, abgr32_t cm, abgr32_t co) {
    auto* ptr = ek_canvas_.vertex_it++;
    ptr->x = pos.x;
    ptr->y = pos.y;
    ptr->u = tex_coord.x;
    ptr->v = tex_coord.y;
    ptr->cm = cm.abgr;
    ptr->co = co.abgr;
}

void write_indices_quad(const uint16_t i0,
                        const uint16_t i1,
                        const uint16_t i2,
                        const uint16_t i3,
                        const uint16_t baseVertex) {
    const uint16_t index = ek_canvas_.vertex_base + baseVertex;
    *(ek_canvas_.index_it++) = index + i0;
    *(ek_canvas_.index_it++) = index + i1;
    *(ek_canvas_.index_it++) = index + i2;
    *(ek_canvas_.index_it++) = index + i2;
    *(ek_canvas_.index_it++) = index + i3;
    *(ek_canvas_.index_it++) = index + i0;
}

void write_indices(const uint16_t* source,
                   uint16_t count,
                   uint16_t baseVertex) {
    const uint16_t index = ek_canvas_.vertex_base + baseVertex;
    for (int i = 0; i < count; ++i) {
        *(ek_canvas_.index_it++) = *(source++) + index;
    }
}

/////

void line(const Vec2f& start, const Vec2f& end, abgr32_t color1, abgr32_t color2, float lineWidth1,
          float lineWidth2) {
    float angle = atan2f(end.y - start.y, end.x - start.x);
    float sn = 0.5f * sinf(angle);
    float cs = 0.5f * cosf(angle);
    float t2sina1 = sn * lineWidth1;
    float t2cosa1 = cs * lineWidth1;
    float t2sina2 = sn * lineWidth2;
    float t2cosa2 = cs * lineWidth2;

    triangles(4, 6);

    auto m1 = state.color[0].scale * color1;
    auto m2 = state.color[0].scale * color2;
    auto co = state.color[0].offset;

    write_vertex(start.x + t2sina1, start.y - t2cosa1, 0, 0, m1, co);
    write_vertex(end.x + t2sina2, end.y - t2cosa2, 1, 0, m2, co);
    write_vertex(end.x - t2sina2, end.y + t2cosa2, 1, 1, m2, co);
    write_vertex(start.x - t2sina1, start.y + t2cosa1, 0, 1, m1, co);

    write_indices_quad();
}

void line(const Vec2f& start,
          const Vec2f& end,
          abgr32_t color,
          float lineWidth) {
    line(start, end, color, color, lineWidth, lineWidth);
}

void line_arc(float x, float y, float r,
              float angle_from, float angle_to,
              float line_width, int segments,
              abgr32_t color_inner, abgr32_t color_outer) {
    auto pi2 = static_cast<float>(Math::pi2);
    float da = pi2 / float(segments);
    float a0 = angle_from;
    auto m1 = state.color[0].scale * color_inner;
    auto m2 = state.color[0].scale * color_outer;
    auto co = state.color[0].offset;
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

void strokeRect(const Rect2f& rc, abgr32_t color, float lineWidth) {
    line({rc.x, rc.y}, {rc.right(), rc.y}, color, lineWidth);
    line({rc.right(), rc.y}, {rc.right(), rc.bottom()}, color, lineWidth);
    line({rc.right(), rc.bottom()}, {rc.x, rc.bottom()}, color, lineWidth);
    line({rc.x, rc.bottom()}, {rc.x, rc.y}, color, lineWidth);
}

void strokeCircle(const CircleF& circle, abgr32_t color, float lineWidth, int segments) {
    const float x = circle.center.x;
    const float y = circle.center.y;
    const float r = circle.radius;

    const float da = Math::fPI2 / (float) segments;
    float a = 0.0f;
    Vec2f pen{x, y - r};
    while (a < Math::fPI2) {
        Vec2f next{x + r * cosf(a), y + r * sinf(a)};
        line(pen, next, color, lineWidth);
        pen = next;
        a += da;
    }
    line(pen, {x, y - r}, color, lineWidth);
}

}

void ek_canvas_resources_create(void) {
    EK_DEBUG("draw2d: create default resources");
    const sg_backend backend = sg_query_backend();
    ek_canvas_.image_empty = ek_gfx_make_color_image(4, 4, 0xFFFFFFFFu);
    ek_canvas_.shader_default = ek_shader_make(draw2d_shader_desc(backend));
    ek_canvas_.shader_alpha_map = ek_shader_make(draw2d_alpha_shader_desc(backend));
    ek_canvas_.shader_solid_color = ek_shader_make(draw2d_color_shader_desc(backend));
    ek_ref_assign_s(sg_image, "empty", ek_canvas_.image_empty);
    ek_ref_assign_s(ek_shader, "draw2d", ek_canvas_.shader_default);
    ek_ref_assign_s(ek_shader, "draw2d_alpha", ek_canvas_.shader_alpha_map);
    ek_ref_assign_s(ek_shader, "draw2d_color", ek_canvas_.shader_solid_color);
}

void ek_canvas_resources_destroy(void) {
    ek_ref_reset_s(sg_image, "empty");
    ek_ref_reset_s(ek_shader, "draw2d");
    ek_ref_reset_s(ek_shader, "draw2d_alpha");
    ek_ref_reset_s(ek_shader, "draw2d_color");
}

void ek_canvas_setup(void) {
    EK_DEBUG("canvas setup");

    //memset(&ek_canvas_, 0, sizeof ek_canvas_);

    ek_canvas_buffers_init(&ek_canvas_.vbs, SG_BUFFERTYPE_VERTEXBUFFER, EK_CANVAS_VERTEX_MAX_COUNT,
                           sizeof(ek_vertex2d));
    ek_canvas_buffers_init(&ek_canvas_.ibs, SG_BUFFERTYPE_INDEXBUFFER, EK_CANVAS_INDEX_MAX_COUNT,
                           sizeof(uint16_t));
    ek::draw2d::context.initialize();

    ek_canvas__reset_stream();

    ek_canvas_resources_create();
}

void ek_canvas_shutdown(void) {
    EK_DEBUG("canvas shutdown");
    ek_canvas_resources_destroy();
    ek::draw2d::context.shutdown();
}

void ek_canvas_new_frame(void) {
    EK_ASSERT(!(ek_canvas_.state & EK_CANVAS_PASS_ACTIVE));
    ek_canvas_.stats = {};
    ek_canvas_buffers_rewind(&ek_canvas_.vbs);
    ek_canvas_buffers_rewind(&ek_canvas_.ibs);
}