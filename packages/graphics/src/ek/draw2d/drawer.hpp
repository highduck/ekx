#pragma once

#include <ek/math/Matrix3x2.hpp>
#include <ek/math/Matrix4.hpp>
#include <ek/math/Color32.hpp>
#include <ek/math/Circle.hpp>
#include <ek/math/Rect.hpp>
#include <ek/gfx.h>

typedef enum ek_canvas_blend {
    EK_CANVAS_BLEND_PMA = 0
} ek_canvas_blend;

typedef struct ek_canvas_frame_stats {
    uint32_t triangles;
    uint32_t draw_calls;
    float fill_area;
} ek_canvas_frame_stats;

typedef struct ek_canvas_scissors {
    union {
        struct {
            int16_t x;
            int16_t y;
            int16_t w;
            int16_t h;
        };
        uint64_t packed;
    };
} ek_canvas_scissors;

typedef struct ek_canvas_batch_state {
    ek_shader shader;
    sg_image image;
    ek_canvas_scissors scissors;
    uint8_t blend;
} ek_canvas_batch_state;

enum {
    EK_CANVAS_PIPELINE_CACHE_MAX_COUNT = 8,
    EK_CANVAS_STACK_MAX_DEPTH = 8,
    EK_CANVAS_BUFFERS_MAX_COUNT = 128,
    EK_CANVAS_INDEX_MAX_COUNT = 0x100000,
    EK_CANVAS_VERTEX_MAX_COUNT = 0x10000,
};

// region Buffers Chain

typedef sg_buffer ek_canvas_buffers_line[EK_CANVAS_BUFFERS_MAX_COUNT];

typedef struct ek_canvas_buffers {
    ek_canvas_buffers_line lines[4];
    uint16_t pos[4];
    // each bucket buffer size
    uint32_t cap[4];
    sg_buffer_type type;
} ek_canvas_buffers;

// endregion

#define EK_CANVAS_INDEX_LIMIT (EK_CANVAS_INDEX_MAX_COUNT - 1)
#define EK_CANVAS_VERTEX_LIMIT (EK_CANVAS_VERTEX_MAX_COUNT - 1)

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
    ek_vertex2d vertex[EK_CANVAS_VERTEX_MAX_COUNT];
    uint16_t index[EK_CANVAS_INDEX_MAX_COUNT];

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
    ek_canvas_pipeline_item pipelines[EK_CANVAS_PIPELINE_CACHE_MAX_COUNT];
    int pipelines_num;

    // Default resources
    ek_shader shader_default;
    ek_shader shader_alpha_map;
    ek_shader shader_solid_color;
    sg_image image_empty;

} ek_canvas_context;

extern ek_canvas_context ek_canvas_;

namespace ek::draw2d {

struct Vertex2D {
    Vec2f position;
    Vec2f uv;
    abgr32_t cm;
    abgr32_t co;
};

struct Context {
    /** Scissors **/

    void pushClipRect(const Rect2f& rc);

    Context& saveScissors();

    void setScissors(const Rect2f& rc);

    Context& popClipRect();

    /** Matrix Transform **/

    Context& save_matrix();

    Context& save_transform();

    Context& restore_transform();

    Context& transform_pivot(Vec2f position, float rotation, Vec2f scale, Vec2f pivot) {
        matrix[0].translate(position.x + pivot.x, position.y + pivot.y)
                .scale(scale.x, scale.y)
                .rotate(rotation)
                .translate(-pivot.x, -pivot.y);
        return *this;
    }

    Context& translate(float tx, float ty);

    Context& translate(const Vec2f& v);

    Context& scale(float sx, float sy);

    Context& scale(const Vec2f& v);

    Context& rotate(float radians);

    Context& concat(const Matrix3x2f& r);

    Context& restore_matrix();

    /** Color Transform **/

    Context& save_color();

    Context& restore_color();

    Context& scaleAlpha(float alpha);

    Context& scaleColor(abgr32_t multiplier);

    Context& concat(abgr32_t scale, abgr32_t offset);

    Context& concat(ColorMod32 color);

    Context& offset_color(abgr32_t offset);

    Context& save_image_rect();

    Context& set_image_rect(float u0, float v0, float du, float dv);

    Context& set_image_rect(const Rect2f& uv_rect);

    Context& restore_image_rect();

    Context& save_image();

    Context& set_empty_image();

    Context& set_image(sg_image image);

    Context& set_image_region(sg_image image = {0},
                              const Rect2f& region = Rect2f{0.0f, 0.0f, 1.0f, 1.0f});

    Context& restore_image();

    Context& pushProgram(ek_shader program_);

    Context& setProgram(ek_shader program_);

    Context& saveProgram();

    Context& restoreProgram();

    // do extra checking and clear states stack
    void check_and_reset_stack();

    void setNextScissors(Rect2i rc);

    void setNextBlending(ek_canvas_blend blend);

    void setNextImage(sg_image image);

    void setNextShader(ek_shader shader);

    void applyNextState();

public:

    void drawUserBatch(sg_pipeline pip, uint32_t images_count);

    void drawBatch();

    void allocTriangles(uint32_t vertex_count, uint32_t index_count);

public:
    // Current and saved state values
    sg_image image[EK_CANVAS_STACK_MAX_DEPTH]; // 1 * 4
    ek_shader shader[EK_CANVAS_STACK_MAX_DEPTH]; // 2 * 4
    Matrix3x2f matrix[EK_CANVAS_STACK_MAX_DEPTH]; // 6 * 4
    ColorMod32 color[EK_CANVAS_STACK_MAX_DEPTH]; // 2 * 4
    Rect2f uv[EK_CANVAS_STACK_MAX_DEPTH]; // 4 * 4
    Rect2f scissors[EK_CANVAS_STACK_MAX_DEPTH]; // 4 * 4

    int image_top;
    int shader_top;
    int matrix_top;
    int color_top;
    int uv_top;
    int scissors_top;

    Matrix4f mvp;
};

extern Context& state;

void begin(Rect2f viewport, const Matrix3x2f& view = Matrix3x2f{}, sg_image renderTarget = {0},
           sg_image depthStencilTarget = {0});

void end();

void write_index(uint16_t index);

void triangles(int vertex_count, int index_count);

void quad(float x, float y, float w, float h);

void quad_rotated(float x, float y, float w, float h);

void quad(float x, float y, float w, float h, abgr32_t color);

void quad(float x, float y, float w, float h, abgr32_t c1, abgr32_t c2, abgr32_t c3, abgr32_t c4);

inline void quad(const Rect2f& rc, abgr32_t color = abgr32_t::one) {
    quad(rc.x, rc.y, rc.width, rc.height, color);
}

void fill_circle(const CircleF& circle, abgr32_t inner_color, abgr32_t outer_color, int segments);

void write_vertex(float x, float y, float u, float v, abgr32_t cm, abgr32_t co);

void write_raw_vertex(const Vec2f& pos, const Vec2f& tex_coord, abgr32_t cm, abgr32_t co);

void write_indices_quad(uint16_t i0,
                        uint16_t i1,
                        uint16_t i2,
                        uint16_t i3,
                        uint16_t base_vertex = 0u);

inline void write_indices_quad(const uint16_t base_index = 0) {
    write_indices_quad(0, 1, 2, 3, base_index);
}

void write_indices(const uint16_t* source, uint16_t count, uint16_t base_vertex = 0);

void line(const Vec2f& start, const Vec2f& end,
          abgr32_t color1, abgr32_t color2,
          float lineWidth1, float lineWidth2);

void line(const Vec2f& start, const Vec2f& end, abgr32_t color = abgr32_t::one, float lineWidth = 1.0f);

void line_arc(float x, float y, float r,
              float angle_from, float angle_to,
              float line_width, int segments,
              abgr32_t color_inner, abgr32_t color_outer);

void strokeRect(const Rect2f& rc, abgr32_t color, float lineWidth);

void strokeCircle(const CircleF& circle, abgr32_t color, float lineWidth, int segments);

}

void ek_canvas_setup(void);

void ek_canvas_shutdown(void);

void ek_canvas_new_frame(void);
