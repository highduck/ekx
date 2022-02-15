#include "canvas.h"

#include "canvas_shader.h"
#include <ek/log.h>
#include <ek/assert.h>

/// region Buffers Chain

#define CANVAS_INDEX_LIMIT (CANVAS_INDEX_MAX_COUNT - 1)
#define CANVAS_VERTEX_LIMIT (CANVAS_VERTEX_MAX_COUNT - 1)

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
    if (index == CANVAS_BUFFERS_MAX_COUNT) {
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
    sg_layout_desc layout = {};
    layout.buffers[0].stride = sizeof(ek_vertex2d);
    layout.attrs[0].offset = offsetof(ek_vertex2d, x);
    layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[1].offset = offsetof(ek_vertex2d, u);
    layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    layout.attrs[2].offset = offsetof(ek_vertex2d, cm);
    layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
    layout.attrs[3].offset = offsetof(ek_vertex2d, co);
    layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    return layout;
}

ek_canvas_context canvas;

static void canvas__resources_create(void) {
    log_debug("canvas: create default resources");
    const sg_backend backend = sg_query_backend();
    res_shader.data[R_SHADER_BLEND] = ek_shader_make(canvas_shader_desc(backend));
    res_shader.data[R_SHADER_ALPHA_MAP] = ek_shader_make(canvas_alpha_shader_desc(backend));
    res_shader.data[R_SHADER_SOLID_COLOR] = ek_shader_make(canvas_color_shader_desc(backend));
    res_shader.rr.num += 4;

    res_image.data[R_IMAGE_EMPTY] = ek_gfx_make_color_image(4, 4, 0xFFFFFFFFu);
    res_image.rr.num += 2;
}

static void canvas__resources_destroy(void) {
    sg_destroy_shader(res_shader.data[R_SHADER_BLEND].shader);
    sg_destroy_shader(res_shader.data[R_SHADER_ALPHA_MAP].shader);
    sg_destroy_shader(res_shader.data[R_SHADER_SOLID_COLOR].shader);
    sg_destroy_image(res_image.data[R_IMAGE_EMPTY]);
}

static void canvas__reset_stack(void) {
    // debug checks
    EK_ASSERT(canvas.image_top == 0);
    EK_ASSERT(canvas.shader_top == 0);
    EK_ASSERT(canvas.scissors_top == 0);
    EK_ASSERT(canvas.matrix_top == 0);
    EK_ASSERT(canvas.color_top == 0);
    EK_ASSERT(canvas.uv_top == 0);

    canvas.image_top = 0;
    canvas.shader_top = 0;
    canvas.scissors_top = 0;
    canvas.matrix_top = 0;
    canvas.color_top = 0;
    canvas.uv_top = 0;
}

static void canvas__reset_stream(void) {
    // just verify that we alloc before write
    canvas.vertex_it = NULL;
    canvas.index_it = NULL;

    canvas.vertex_end = canvas.vertex;
    canvas.index_end = canvas.index;

    canvas.vertex_num = 0;
    canvas.index_num = 0;
}


/** Scissors **/

void canvas_save_scissors(void) {
    ++canvas.scissors_top;
    EK_ASSERT(canvas.scissors_top != CANVAS_STACK_DEPTH);
    canvas.scissors[canvas.scissors_top] = canvas.scissors[0];
}

void canvas_set_scissors(const rect_t rc) {
    canvas.scissors[0] = rc;
    canvas.state |= EK_CANVAS_CHECK_SCISSORS;
}

void canvas_push_scissors(const rect_t rc) {
    canvas_save_scissors();
    canvas_set_scissors(rect_clamp_bounds(canvas.scissors[0], rc));
}

void canvas_pop_scissors(void) {
    EK_ASSERT(canvas.scissors_top != 0);
    canvas.scissors[0] = canvas.scissors[canvas.scissors_top--];
    canvas.state |= EK_CANVAS_CHECK_SCISSORS;
}


/** Matrix Transform **/

void canvas_save_matrix(void) {
    ++canvas.matrix_top;
    EK_ASSERT(canvas.matrix_top != CANVAS_STACK_DEPTH);
    canvas.matrix[canvas.matrix_top] = canvas.matrix[0];
}

void canvas_save_transform(void) {
    canvas_save_matrix();
    canvas_save_color();
}

void canvas_restore_transform(void) {
    canvas_restore_matrix();
    canvas_restore_color();
}

void canvas_translate(const vec2_t v) {
    mat3x2_translate(canvas.matrix, v);
}

void canvas_scale(const vec2_t v) {
    mat3x2_scale(canvas.matrix, v);
}

void canvas_rotate(float radians) {
    mat3x2_rotate(canvas.matrix, radians);
}

void canvas_concat_matrix(const mat3x2_t r) {
    *canvas.matrix = mat3x2_mul(*canvas.matrix, r);
}

void canvas_restore_matrix(void) {
    EK_ASSERT(canvas.matrix_top != 0);
    *canvas.matrix = canvas.matrix[canvas.matrix_top--];
}


/** Color Transform **/

void canvas_save_color(void) {
    ++canvas.color_top;
    EK_ASSERT(canvas.color_top != CANVAS_STACK_DEPTH);
    canvas.color[canvas.color_top] = canvas.color[0];
}

void canvas_restore_color(void) {
    EK_ASSERT(canvas.color_top != 0);
    canvas.color[0] = canvas.color[canvas.color_top--];
}

void canvas_scale_alpha(float alpha) {
    canvas.color[0].scale.a = u8_norm_mul(canvas.color[0].scale.a, (uint8_t) (alpha * 255));
}

void canvas_scale_color(color_t multiplier) {
    canvas.color[0].scale = mul_color(canvas.color[0].scale, multiplier);
}

void canvas_concat_color(const color2_t color) {
    color2_concat(canvas.color, color.scale, color.offset);
}

void canvas_offset_color(color_t offset) {
    color2_add(canvas.color, offset);
}

/** STATES **/

void canvas_save_image_rect(void) {
    ++canvas.uv_top;
    EK_ASSERT(canvas.uv_top != CANVAS_STACK_DEPTH);
    canvas.uv[canvas.uv_top] = canvas.uv[0];
}

void canvas_set_image_rect(const rect_t uv_rect) {
    canvas.uv[0] = uv_rect;
}

void canvas_restore_image_rect(void) {
    EK_ASSERT(canvas.uv_top != 0);
    canvas.uv[0] = canvas.uv[canvas.uv_top--];
}

void canvas_save_image(void) {
    ++canvas.image_top;
    EK_ASSERT(canvas.image_top != CANVAS_STACK_DEPTH);
    canvas.image[canvas.image_top] = canvas.image[0];
}

void canvas_set_empty_image(void) {
    canvas.image[0] = res_image.data[R_IMAGE_EMPTY];
    canvas.state |= EK_CANVAS_CHECK_IMAGE;
    canvas_set_image_rect(rect_01());
}

void canvas_set_image(sg_image image_) {
    canvas.image[0] = image_;
    canvas.state |= EK_CANVAS_CHECK_IMAGE;
}

void canvas_set_image_region(sg_image image_, const rect_t region) {
    canvas.image[0] = image_.id ? image_ : res_image.data[R_IMAGE_EMPTY];
    canvas.state |= EK_CANVAS_CHECK_IMAGE;
    canvas.uv[0] = region;
}

void canvas_restore_image(void) {
    EK_ASSERT(canvas.image_top != 0);
    canvas.image[0] = canvas.image[canvas.image_top--];
    canvas.state |= EK_CANVAS_CHECK_IMAGE;
}

void canvas_push_program(const ek_shader program_) {
    canvas_save_program();
    canvas_set_program(program_);
}

void canvas_set_program(const ek_shader program_) {
    canvas.shader[0] = program_.shader.id ? program_ : res_shader.data[R_SHADER_BLEND];
    canvas.state |= EK_CANVAS_CHECK_SHADER;
}

void canvas_save_program(void) {
    ++canvas.shader_top;
    EK_ASSERT(canvas.shader_top != CANVAS_STACK_DEPTH);
    canvas.shader[canvas.shader_top] = canvas.shader[0];
}

void canvas_restore_program(void) {
    EK_ASSERT(canvas.shader_top != 0);
    canvas.shader[0] = canvas.shader[canvas.shader_top--];
    canvas.state |= EK_CANVAS_CHECK_SHADER;
}

/*** drawings ***/

void canvas_quad(float x, float y, float w, float h) {
    canvas_triangles(4, 6);

    const color_t cm = canvas.color[0].scale;
    const color_t co = canvas.color[0].offset;
    canvas_write_vertex(x, y, 0, 0.0f, cm, co);
    canvas_write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    canvas_write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    canvas_write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    canvas_write_quad_indices();
}

void canvas_quad_color(float x, float y, float w, float h, color_t color) {
    canvas_triangles(4, 6);

    const color_t cm = mul_color(canvas.color[0].scale, color);
    const color_t co = canvas.color[0].offset;
    canvas_write_vertex(x, y, 0, 0.0f, cm, co);
    canvas_write_vertex(x + w, y, 1.0f, 0.0f, cm, co);
    canvas_write_vertex(x + w, y + h, 1.0f, 1.0f, cm, co);
    canvas_write_vertex(x, y + h, 0.0f, 1.0f, cm, co);

    canvas_write_quad_indices();
}

void canvas_quad_color4(float x, float y, float w, float h, color_t c1, color_t c2, color_t c3, color_t c4) {
    canvas_triangles(4, 6);

    const color_t cm = canvas.color[0].scale;
    const color_t co = canvas.color[0].offset;
    c1 = mul_color(cm, c1);
    c2 = mul_color(cm, c2);
    c3 = mul_color(cm, c3);
    c4 = mul_color(cm, c4);
    canvas_write_vertex(x, y, 0, 0.0f, c1, co);
    canvas_write_vertex(x + w, y, 1.0f, 0.0f, c2, co);
    canvas_write_vertex(x + w, y + h, 1.0f, 1.0f, c3, co);
    canvas_write_vertex(x, y + h, 0.0f, 1.0f, c4, co);

    canvas_write_quad_indices();
}

void canvas_quad_rotated(float x, float y, float w, float h) {
    canvas_triangles(4, 6);

    const color_t cm = canvas.color[0].scale;
    const color_t co = canvas.color[0].offset;
    canvas_write_vertex(x, y, 0, 1, cm, co);
    canvas_write_vertex(x + w, y, 0, 0, cm, co);
    canvas_write_vertex(x + w, y + h, 1, 0, cm, co);
    canvas_write_vertex(x, y + h, 1, 1, cm, co);

    canvas_write_quad_indices();
}

void canvas_fill_rect(const rect_t rc, color_t color) {
    canvas_quad_color(rc.x, rc.y, rc.w, rc.h, color);
}

// This function should be moved to the dedicated `indexed draw` mode
void canvas_fill_circle(const vec3_t circle, color_t inner_color, color_t outer_color, int segments) {
    canvas_triangles(1 + segments, 3 * segments);

    const float x = circle.x;
    const float y = circle.y;
    const float r = circle.z;

    const color_t co = canvas.color[0].offset;
    const color_t inner_cm = mul_color(canvas.color[0].scale, inner_color);
    const color_t outer_cm = mul_color(canvas.color[0].scale, outer_color);
    canvas_write_vertex(x, y, 0.0f, 0.0f, inner_cm, co);

    const float da = MATH_TAU / (float) segments;
    float a = 0.0f;
    while (a < MATH_TAU) {
        canvas_write_vertex(x + r * cosf(a), y + r * sinf(a), 1, 1, outer_cm, co);
        a += da;
    }

    const uint16_t end = segments - 1;
    for (uint16_t i = 0; i < end; ++i) {
        canvas_write_index(0u);
        canvas_write_index(i + 1u);
        canvas_write_index(i + 2u);
    }
    canvas_write_index(0u);
    canvas_write_index(segments);
    canvas_write_index(1u);
}

/////

void canvas_line_ex(const vec2_t start, const vec2_t end, color_t color1, color_t color2, float width1, float width2) {
    canvas_triangles(4, 6);

    const float angle = atan2f(end.y - start.y, end.x - start.x);
    const float sn = 0.5f * sinf(angle);
    const float cs = 0.5f * cosf(angle);
    const float t2sina1 = sn * width1;
    const float t2cosa1 = cs * width1;
    const float t2sina2 = sn * width2;
    const float t2cosa2 = cs * width2;

    const color_t m1 = mul_color(canvas.color[0].scale, color1);
    const color_t m2 = mul_color(canvas.color[0].scale, color2);
    const color_t co = canvas.color[0].offset;

    canvas_write_vertex(start.x + t2sina1, start.y - t2cosa1, 0, 0, m1, co);
    canvas_write_vertex(end.x + t2sina2, end.y - t2cosa2, 1, 0, m2, co);
    canvas_write_vertex(end.x - t2sina2, end.y + t2cosa2, 1, 1, m2, co);
    canvas_write_vertex(start.x - t2sina1, start.y + t2cosa1, 0, 1, m1, co);

    canvas_write_quad_indices();
}

void canvas_line(const vec2_t start, const vec2_t end, color_t color, float line_width) {
    canvas_line_ex(start, end, color, color, line_width, line_width);
}

void canvas_line_arc(float x, float y, float r,
                     float angle_from, float angle_to,
                     float line_width, int segments,
                     color_t color_inner, color_t color_outer) {
    const float da = MATH_TAU / (float) segments;
    const color_t m1 = mul_color(canvas.color[0].scale, color_inner);
    const color_t m2 = mul_color(canvas.color[0].scale, color_outer);
    const color_t co = canvas.color[0].offset;
    const float hw = line_width / 2.0f;
    const float r0 = r - hw;
    const float r1 = r + hw;

    float a0 = angle_from;
    while (a0 < angle_to) {

        canvas_triangles(4, 6);
        const float a1 = fminf(angle_to, a0 + da);
        const float cs0 = cosf(a0);
        const float sn0 = sinf(a0);
        const float cs1 = cosf(a1);
        const float sn1 = sinf(a1);

        canvas_write_vertex(x + r1 * cs0, y + r1 * sn0, 0, 0, m2, co);
        canvas_write_vertex(x + r1 * cs1, y + r1 * sn1, 1, 0, m2, co);
        canvas_write_vertex(x + r0 * cs1, y + r0 * sn1, 1, 1, m1, co);
        canvas_write_vertex(x + r0 * cs0, y + r0 * sn0, 0, 1, m1, co);

        canvas_write_quad_indices();

        a0 += da;
    }
}

void canvas_stroke_rect(const rect_t rc, color_t color, float lineWidth) {
    const float r = rc.x + rc.w;
    const float b = rc.y + rc.h;
    canvas_line(vec2(rc.x, rc.y), vec2(r, rc.y), color, lineWidth);
    canvas_line(vec2(r, rc.y), vec2(r, b), color, lineWidth);
    canvas_line(vec2(r, b), vec2(rc.x, b), color, lineWidth);
    canvas_line(vec2(rc.x, b), vec2(rc.x, rc.y), color, lineWidth);
}

void canvas_stroke_circle(const vec3_t circle, color_t color, float lineWidth, int segments) {
    const float x = circle.x;
    const float y = circle.y;
    const float r = circle.z;

    const float da = MATH_TAU / (float) segments;
    float a = da;
    vec2_t pen = vec2(x + r, y);
    while (a < MATH_TAU) {
        const vec2_t next = vec2(x + r * cosf(a), y + r * sinf(a));
        canvas_line(pen, next, color, lineWidth);
        pen = next;
        a += da;
    }
    canvas_line(pen, vec2(x + r, y), color, lineWidth);
}

static float triangle_area(const ek_vertex2d* vertices, const uint16_t* indices, int count) {
    float sum = 0.0f;
    for (int i = 0; i < count;) {
        const ek_vertex2d a = vertices[indices[i++]];
        const ek_vertex2d b = vertices[indices[i++]];
        const ek_vertex2d c = vertices[indices[i++]];
        sum += a.x * b.y + b.x * c.y + c.x * a.y - a.x * c.y - b.x * a.y - c.x * b.y;
    }
    return sum / 2.0f;
}

static sg_pipeline create_pipeline_for_cache(sg_shader shader, bool useRenderTarget, bool depthStencil) {
    sg_pipeline_desc pip_desc = {};
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
    pip_desc.label = "canvas-pipeline";
    return sg_make_pipeline(&pip_desc);
}

static sg_pipeline get_pipeline(sg_shader shader, bool useRenderTarget, bool depthStencilPass) {
    uint32_t mode = 0;
    if (useRenderTarget) {
        mode |= 0x1;
    }
    if (depthStencilPass) {
        mode |= 0x2;
    }
    for (int i = 0; i < canvas.pipelines_num; ++i) {
        ek_canvas_pipeline_item item = canvas.pipelines[i];
        if (item.shader.id == shader.id && item.mode == mode) {
            return item.pipeline;
        }
    }
    EK_ASSERT(canvas.pipelines_num != CANVAS_PIPELINE_LOOKUP_SIZE);
    ek_canvas_pipeline_item item;
    item.mode = mode;
    item.shader = shader;
    item.pipeline = create_pipeline_for_cache(shader, useRenderTarget, depthStencilPass);

    canvas.pipelines[canvas.pipelines_num++] = item;
    log_debug("pipelines: %d", canvas.pipelines_num);

    return item.pipeline;
}

static void canvas_set_next_scissors(const i16rect_t rc) {
    if (rc.value != canvas.curr.scissors.value) {
        canvas.state |= EK_CANVAS_STATE_CHANGED;
    }
    canvas.next.scissors = rc;
}

static void canvas_set_next_blending(ek_canvas_blend blend) {
    if (canvas.curr.blend != blend) {
        canvas.state |= EK_CANVAS_STATE_CHANGED;
    }
    canvas.next.blend = blend;
}

static void canvas_set_next_image(sg_image image_) {
    if (canvas.curr.image.id != image_.id) {
        canvas.state |= EK_CANVAS_STATE_CHANGED;
    }
    canvas.next.image = image_;
}

static void canvas_set_next_shader(ek_shader shader_) {
    if (canvas.curr.shader.shader.id != shader_.shader.id) {
        canvas.state |= EK_CANVAS_STATE_CHANGED;
    }
    canvas.next.shader = shader_;
}

static void canvas_apply_next_state(void) {
    if (canvas.state & EK_CANVAS_STATE_CHANGED) {
        canvas.curr = canvas.next;
        canvas.state ^= EK_CANVAS_STATE_CHANGED;
    }
}

void canvas_triangles(uint32_t vertex_count, uint32_t index_count) {
    if (canvas.state & EK_CANVAS_CHECK_MASK) {
        if (canvas.state & EK_CANVAS_CHECK_IMAGE) {
            canvas_set_next_image(canvas.image[0]);
            canvas.state ^= EK_CANVAS_CHECK_IMAGE;
        }
        if (canvas.state & EK_CANVAS_CHECK_SHADER) {
            canvas_set_next_shader(canvas.shader[0]);
            canvas.state ^= EK_CANVAS_CHECK_SHADER;
        }
        if (canvas.state & EK_CANVAS_CHECK_SCISSORS) {
            canvas_set_next_scissors((i16rect_t) {
                    (int16_t) canvas.scissors[0].x,
                    (int16_t) canvas.scissors[0].y,
                    (int16_t) canvas.scissors[0].w,
                    (int16_t) canvas.scissors[0].h,
            });
            canvas.state ^= EK_CANVAS_CHECK_SCISSORS;
        }
    }

    if ((canvas.state & EK_CANVAS_STATE_CHANGED) || (canvas.vertex_num + vertex_count) > CANVAS_VERTEX_LIMIT) {
        canvas_draw_batch();
        canvas_apply_next_state();
    }

    canvas.vertex_it = canvas.vertex_end;
    canvas.index_it = canvas.index_end;

    canvas.vertex_end += vertex_count;
    canvas.index_end += index_count;

    canvas.vertex_base = canvas.vertex_num;
    canvas.vertex_num += vertex_count;
    canvas.index_num += index_count;
}

int canvas__prepare_draw(uint32_t images_num) {
    int index_num = (int) canvas.index_num;
    if (index_num == 0) {
        return 0;
    }

    const uint32_t vertexDataSize = (uint32_t) (canvas.vertex_num * sizeof(ek_vertex2d));
    const uint32_t indexDataSize = index_num << 1u;
    const sg_buffer vb = ek_canvas_buffers_get(&canvas.vbs, vertexDataSize);
    const sg_buffer ib = ek_canvas_buffers_get(&canvas.ibs, indexDataSize);
    EK_ASSERT(vb.id != 0);
    EK_ASSERT(ib.id != 0);
    sg_update_buffer(vb, &(sg_range) {canvas.vertex, vertexDataSize});
    sg_update_buffer(ib, &(sg_range) {canvas.index, indexDataSize});

    canvas.bind.vertex_buffers[0] = vb;
    canvas.bind.index_buffer = ib;
    canvas.bind.fs_images[0].id = images_num == 1 ? canvas.curr.image.id : SG_INVALID_ID;

    return index_num;
}

void canvas__draw(int index_num) {
    sg_apply_bindings(&canvas.bind);

    const i16rect_t rc = canvas.curr.scissors;
    sg_apply_scissor_rect(rc.x, rc.y, rc.w, rc.h, true);

    sg_draw(0, index_num, 1);

#ifndef NDEBUG
    // TODO: add mvp matrix and viewport to get real pixels
    canvas.stats.fill_area += triangle_area(canvas.vertex, canvas.index, index_num);
#endif
    canvas.stats.triangles += index_num / 3;
    ++canvas.stats.draw_calls;

    canvas__reset_stream();
}

void canvas_draw_batch(void) {
    const int index_num = canvas__prepare_draw(canvas.curr.shader.images_num);
    if (index_num) {
        const sg_image fb_color = canvas.render_target_color.id ? canvas.render_target_color
                                                                : canvas.framebuffer_color;
        const sg_image fb_depth = canvas.render_target_depth.id ? canvas.render_target_depth
                                                                : canvas.framebuffer_depth;

        const sg_pipeline pip = get_pipeline(canvas.curr.shader.shader, fb_color.id != 0, fb_depth.id != 0);
        if (pip.id != canvas.pipeline.id) {
            canvas.pipeline = pip;
            sg_apply_pipeline(pip);
            sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, SG_RANGE_REF(canvas.mvp));
        }
        canvas__draw(index_num);
    }
}

void canvas_draw_batch_pipeline(sg_pipeline pip, uint32_t images_count) {
    const int index_num = canvas__prepare_draw(images_count);
    if (index_num) {
        // reset current pipeline
        canvas.pipeline = pip;
        canvas__draw(index_num);
    }
}

void canvas_setup(void) {
    log_debug("canvas setup");

    //memset(&ek_canvas_, 0, sizeof ek_canvas_);

    ek_canvas_buffers_init(&canvas.vbs, SG_BUFFERTYPE_VERTEXBUFFER, CANVAS_VERTEX_MAX_COUNT,
                           sizeof(ek_vertex2d));
    ek_canvas_buffers_init(&canvas.ibs, SG_BUFFERTYPE_INDEXBUFFER, CANVAS_INDEX_MAX_COUNT,
                           sizeof(uint16_t));
    canvas__reset_stream();
    canvas__resources_create();
}

void canvas_shutdown(void) {
    log_debug("canvas shutdown");
    canvas__resources_destroy();
}

void canvas_new_frame(void) {
    EK_ASSERT(!(canvas.state & EK_CANVAS_PASS_ACTIVE));
    canvas.stats = (ek_canvas_frame_stats) {};
    ek_canvas_buffers_rewind(&canvas.vbs);
    ek_canvas_buffers_rewind(&canvas.ibs);
}

void canvas_begin(float w, float h) {
    canvas_begin_ex(rect_wh(w, h), mat3x2_identity(), (sg_image) {0}, (sg_image) {0});
}

void canvas_begin_ex(const rect_t viewport, const mat3x2_t view, sg_image renderTarget, sg_image depthStencilTarget) {
    EK_ASSERT(!(canvas.state & EK_CANVAS_PASS_ACTIVE));
    // reset all bits and set Active mode / dirty state flag
    canvas.state = EK_CANVAS_PASS_ACTIVE | EK_CANVAS_STATE_CHANGED;

    canvas.image[0] = res_image.data[R_IMAGE_EMPTY];
    canvas.shader[0] = res_shader.data[R_SHADER_BLEND];
    canvas.scissors[0] = viewport;
    canvas.matrix[0] = mat3x2_identity();
    canvas.color[0] = color2_identity();
    canvas.uv[0] = rect_01();

    canvas.curr = (ek_canvas_batch_state) {};
    canvas.next.shader = canvas.shader[0];
    canvas.next.image = canvas.image[0];
    canvas.next.scissors = (i16rect_t) {
            (int16_t) viewport.x,
            (int16_t) viewport.y,
            (int16_t) viewport.w,
            (int16_t) viewport.h,
    };
    canvas.pipeline.id = SG_INVALID_ID;

    canvas.render_target_color = renderTarget;
    canvas.render_target_depth = depthStencilTarget;

    float proj_y = viewport.y;
    float proj_h = viewport.h;
#ifndef __APPLE__
    // if not Metal backend - we should flip viewport for render-target
    if (canvas.render_target_color.id) {
        proj_y = RECT_B(viewport);
        proj_h = -viewport.h;
    }
#endif
    const mat4_t proj = mat4_orthographic_2d(viewport.x, proj_y, viewport.w, proj_h, -1, 1);
    canvas.mvp = mat4_mul_mat3x2(proj, view);
}

void canvas_end(void) {
    EK_ASSERT(canvas.state & EK_CANVAS_PASS_ACTIVE);
    canvas_draw_batch();
    canvas__reset_stack();
    canvas.state ^= EK_CANVAS_PASS_ACTIVE;
}

void canvas_write_vertex(float x, float y, float u, float v, color_t cm, color_t co) {
    // could be cached before draw2d
    const mat3x2_t m = canvas.matrix[0];
    const rect_t uv = canvas.uv[0];

    ek_vertex2d* ptr = canvas.vertex_it++;
    ptr->x = x * m.a + y * m.c + m.tx;
    ptr->y = x * m.b + y * m.d + m.ty;
    ptr->u = uv.x + u * uv.w;
    ptr->v = uv.y + v * uv.h;
    ptr->cm = cm.value;
    ptr->co = co.value;
}

void canvas_write_raw_vertex(const vec2_t pos, const vec2_t tex_coord, color_t cm, color_t co) {
    ek_vertex2d* ptr = canvas.vertex_it++;
    ptr->x = pos.x;
    ptr->y = pos.y;
    ptr->u = tex_coord.x;
    ptr->v = tex_coord.y;
    ptr->cm = cm.value;
    ptr->co = co.value;
}

void canvas_write_index(uint16_t index) {
    *(canvas.index_it++) = canvas.vertex_base + index;
}

void canvas_write_quad_indices(void) {
    const uint16_t index = canvas.vertex_base;
    *(canvas.index_it++) = index;
    *(canvas.index_it++) = index + 1;
    *(canvas.index_it++) = index + 2;
    *(canvas.index_it++) = index + 2;
    *(canvas.index_it++) = index + 3;
    *(canvas.index_it++) = index;
}

void canvas_write_indices(const uint16_t* source, uint16_t count, uint16_t vertex_base) {
    const uint16_t index = canvas.vertex_base + vertex_base;
    for (int i = 0; i < count; ++i) {
        *(canvas.index_it++) = *(source++) + index;
    }
}
