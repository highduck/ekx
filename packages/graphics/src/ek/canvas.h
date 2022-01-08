#ifndef EK_CANVAS_H
#define EK_CANVAS_H

#include <ek/gfx.h>
#include <ek/math.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ek_canvas_blend {
    EK_CANVAS_BLEND_PMA = 0
} ek_canvas_blend;

typedef struct ek_canvas_frame_stats {
    uint32_t triangles;
    uint32_t draw_calls;
    float fill_area;
} ek_canvas_frame_stats;

typedef struct ek_canvas_batch_state {
    ek_shader shader;
    sg_image image;
    rect_i16_t scissors;
    uint8_t blend;
} ek_canvas_batch_state;

enum {
    CANVAS_PIPELINE_LOOKUP_SIZE = 8,
    CANVAS_STACK_DEPTH = 8,
    CANVAS_BUFFERS_MAX_COUNT = 128,
    CANVAS_INDEX_MAX_COUNT = 0x100000,
    CANVAS_VERTEX_MAX_COUNT = 0x10000,
};

// region Buffers Chain

typedef sg_buffer ek_canvas_buffers_line[CANVAS_BUFFERS_MAX_COUNT];

typedef struct ek_canvas_buffers {
    ek_canvas_buffers_line lines[4];
    uint16_t pos[4];
    // each bucket buffer size
    uint32_t cap[4];
    sg_buffer_type type;
} ek_canvas_buffers;

// endregion

sg_layout_desc ek_vertex2d_layout_desc(void);

typedef struct ek_vertex2d {
    float x;
    float y;
    float u;
    float v;
    uint32_t cm;
    uint32_t co;
} ek_vertex2d;

typedef enum ek_canvas_state_flags {
    EK_CANVAS_CHECK_IMAGE = 0x1,
    EK_CANVAS_CHECK_SHADER = 0x2,
    EK_CANVAS_CHECK_SCISSORS = 0x4,
    EK_CANVAS_STATE_CHANGED = 0x8,
    EK_CANVAS_PASS_ACTIVE = 0x10,
    EK_CANVAS_CHECK_MASK = EK_CANVAS_CHECK_SCISSORS | EK_CANVAS_CHECK_SHADER | EK_CANVAS_CHECK_IMAGE,
    _EK_CANVAS_STATE_FLAGS_FORCE_U32 = 0x7FFFFFFFu
} ek_canvas_state_flags;

typedef struct ek_canvas_pipeline_item {
    sg_shader shader;
    uint32_t mode;
    sg_pipeline pipeline;
} ek_canvas_pipeline_item;

typedef struct ek_canvas_context {
    // Scratch memory
    ek_vertex2d vertex[CANVAS_VERTEX_MAX_COUNT];
    uint16_t index[CANVAS_INDEX_MAX_COUNT];

    // Buffers
    ek_canvas_buffers ibs;
    ek_canvas_buffers vbs;

    // iterators and next position in scratch memory (vertex and index data)
    ek_vertex2d* vertex_it;
    uint16_t* index_it;

    ek_vertex2d* vertex_end;
    uint16_t* index_end;

    uint32_t vertex_num;
    uint32_t index_num;

    uint16_t vertex_base;

    // Stats during frame
    ek_canvas_frame_stats stats;

    // Current begin/end pass state
    // Checking what states could be potentially changed
    uint32_t state;

    // Batch state tracking
    ek_canvas_batch_state curr;
    ek_canvas_batch_state next;

    // render destination or offscreen framebuffer (framebuffer target could be also render target or null as default)
    sg_image render_target_color;
    sg_image render_target_depth;
    sg_image framebuffer_color;
    sg_image framebuffer_depth;

    sg_pipeline pipeline;
    sg_bindings bind;

    // Pipeline cache
    ek_canvas_pipeline_item pipelines[CANVAS_PIPELINE_LOOKUP_SIZE];
    int pipelines_num;

    // Default resources
    ek_shader shader_default;
    ek_shader shader_alpha_map;
    ek_shader shader_solid_color;
    sg_image image_empty;

    // Current and saved state values
    sg_image image[CANVAS_STACK_DEPTH]; // 1 * 4
    ek_shader shader[CANVAS_STACK_DEPTH]; // 2 * 4
    mat3x2_t matrix[CANVAS_STACK_DEPTH]; // 6 * 4
    color2_t color[CANVAS_STACK_DEPTH]; // 2 * 4
    rect_t uv[CANVAS_STACK_DEPTH]; // 4 * 4
    rect_t scissors[CANVAS_STACK_DEPTH]; // 4 * 4

    int image_top;
    int shader_top;
    int matrix_top;
    int color_top;
    int uv_top;
    int scissors_top;

    mat4_t mvp;

} ek_canvas_context;

extern ek_canvas_context canvas;

/** Scissors **/

void canvas_push_scissors(rect_t rc);

void canvas_save_scissors(void);

void canvas_set_scissors(rect_t rc);

void canvas_pop_scissors(void);


/** Matrix Transform **/

void canvas_save_matrix(void);

void canvas_save_transform(void);

void canvas_restore_transform(void);

inline void canvas_transform_pivot(vec2_t pos, float rotation, vec2_t scale, vec2_t pivot) {
    mat3x2_transform_pivot(canvas.matrix, pos, rotation, scale, pivot);
}

void canvas_translate(vec2_t delta);

void canvas_scale(vec2_t scale);

void canvas_rotate(float radians);

void canvas_concat_matrix(mat3x2_t matrix);

void canvas_restore_matrix(void);

/** Color Transform **/

void canvas_save_color(void);

void canvas_restore_color(void);

void canvas_scale_alpha(float alpha);

void canvas_scale_color(rgba_t multiplier);

void canvas_concat_color(color2_t color);

void canvas_offset_color(rgba_t offset);

void canvas_save_image_rect(void);

//void canvas_set_image_rect(float u0, float v0, float du, float dv);

void canvas_set_image_rect(rect_t uv_rect);

void canvas_restore_image_rect(void);

void canvas_save_image(void);

void canvas_set_empty_image(void);

void canvas_set_image(sg_image image);

void canvas_set_image_region(sg_image image, rect_t region);

void canvas_restore_image(void);

void canvas_push_program(ek_shader program_);

void canvas_set_program(ek_shader program_);

void canvas_save_program(void);

void canvas_restore_program(void);

void canvas_quad(float x, float y, float w, float h);

void canvas_quad_rotated(float x, float y, float w, float h);

void canvas_quad_color(float x, float y, float w, float h, rgba_t color);

void canvas_quad_color4(float x, float y, float w, float h, rgba_t c1, rgba_t c2, rgba_t c3, rgba_t c4);

void canvas_fill_rect(rect_t rc, rgba_t color);

void canvas_fill_circle(circle_t circle, rgba_t inner_color, rgba_t outer_color, int segments);

void canvas_line_ex(vec2_t start, vec2_t end, rgba_t color1, rgba_t color2, float lineWidth1, float lineWidth2);

void canvas_line(vec2_t start, vec2_t end, rgba_t color, float line_width);

void canvas_line_arc(float x, float y, float r,
                     float angle_from, float angle_to,
                     float line_width, int segments,
                     rgba_t color_inner, rgba_t color_outer);

void canvas_stroke_rect(rect_t rc, rgba_t color, float lineWidth);

void canvas_stroke_circle(circle_t circle, rgba_t color, float lineWidth, int segments);

void canvas_setup(void);

void canvas_shutdown(void);

void canvas_new_frame(void);

void canvas_begin(float w, float h);

void canvas_begin_ex(rect_t viewport, mat3x2_t view, sg_image renderTarget, sg_image depthStencilTarget);

void canvas_end(void);

void canvas_triangles(uint32_t vertex_count, uint32_t index_count);

void canvas_draw_batch_pipeline(sg_pipeline pip, uint32_t images_count);

void canvas_draw_batch(void);

void canvas_write_vertex(float x, float y, float u, float v, rgba_t cm, rgba_t co);

void canvas_write_raw_vertex(vec2_t pos, vec2_t tex_coord, rgba_t cm, rgba_t co);

void canvas_write_index(uint16_t index);

void canvas_write_quad_indices(void);

void canvas_write_indices(const uint16_t* source, uint16_t count, uint16_t base_vertex);

#ifdef __cplusplus
}
#endif

#endif // EK_CANVAS_H
