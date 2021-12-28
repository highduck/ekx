#pragma once

#include <ek/ds/Array.hpp>
#include <ek/ds/Hash.hpp>
#include <ek/util/NoCopyAssign.hpp>
#include <ek/util/Res.hpp>
#include <ek/math/Matrix3x2.hpp>
#include <ek/math/Matrix4.hpp>
#include <ek/math/Color32.hpp>
#include <ek/math/Circle.hpp>
#include <ek/math/Rect.hpp>
#include <ek/graphics/graphics.hpp>

namespace ek::draw2d {

struct FrameStats {
    uint32_t triangles = 0u;
    uint32_t drawCalls = 0u;
    float fillArea = 0.0f;
};

struct Vertex2D {
    Vec2f position;
    Vec2f uv;
    abgr32_t cm;
    abgr32_t co;

    static sg_layout_desc layout();
};

enum class BlendMode : uint8_t {
    PremultipliedAlpha = 0
};

struct BatchState {
    sg_shader shader = {0};
    sg_image image = {0};
    Rect2i scissors{};
    BlendMode blend = BlendMode::PremultipliedAlpha;
    uint8_t shader_images_count = 0;

    bool operator==(const BatchState& a) const {
        return (blend == a.blend && shader.id == a.shader.id && image.id == a.image.id);
    }

    bool operator!=(const BatchState& a) const {
        return (blend != a.blend || shader.id != a.shader.id || image.id != a.image.id);
    }
};

class BufferChain {
public:
    BufferChain(sg_buffer_type type, uint32_t elementsMaxCount, uint32_t elementMaxSize) :
            type_{type},
            caps{0x10 * elementMaxSize,
                 0x100 * elementMaxSize,
                 0x400 * elementMaxSize,
                 elementsMaxCount * elementMaxSize} {
    }

    uint32_t calcSizeBucket(uint32_t requiredSize) {
        if (requiredSize < caps[0]) {
            return 0;
        } else if (requiredSize < caps[1]) {
            return 1;
        } else if (requiredSize < caps[2]) {
            return 2;
        }
        return 3;
    }

    sg_buffer nextBuffer(uint32_t requiredSize) {
        sg_buffer buf = {};
        const auto index = calcSizeBucket(requiredSize);
        auto& v = buffers_[index];
        auto position = pos[index];
        if (position >= v.size()) {
            sg_buffer_desc desc{};
            desc.usage = SG_USAGE_STREAM;
            desc.type = (sg_buffer_type) type_;
            desc.size = caps[index];
            buf = sg_make_buffer(&desc);
            EK_ASSERT(buf.id != 0);
            v.push_back(buf);
        } else {
            buf = v[position];
        }
        ++position;
        pos[index] = position;
        return buf;
    }

    void nextFrame() {
        resetPositions();
    }

    void resetPositions() {
        pos[0] = 0;
        pos[1] = 0;
        pos[2] = 0;
        pos[3] = 0;
    }

    void disposeBuffers() {
        for (auto& chain: buffers_) {
            for (sg_buffer buffer: chain) {
                sg_destroy_buffer(buffer);
            }
            chain.resize(0);
        }
    }

    void reset() {
        disposeBuffers();
        resetPositions();
    }

    ~BufferChain() {
        disposeBuffers();
    }

private:
    sg_buffer_type type_;
    Array<sg_buffer> buffers_[4]{};
    uint16_t pos[4] = {0, 0, 0, 0};
    // each bucket buffer size
    uint32_t caps[4];
};

struct Context : private NoCopyAssign {

    Context();

    ~Context();

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
        matrix.translate(position.x + pivot.x, position.y + pivot.y)
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
    void finish();

    void setNextScissors(Rect2i rc);

    void setNextBlending(BlendMode blending);

    void setNextImage(sg_image image);

    void setNextShader(sg_shader shader, uint8_t images_count);

    void applyNextState();

public:

    void drawUserBatch(sg_pipeline pip, uint32_t images_count);

    void drawBatch();

    void allocTriangles(uint32_t vertex_count, uint32_t index_count);

    sg_pipeline getPipeline(sg_shader shader, bool useRenderTarget, bool depthStencilPass);

public:

    constexpr static int MaxIndex = 0xFFFFF;
    constexpr static int MaxVertex = 0xFFFF;

    // Default resources
    ek_shader defaultShader = {};
    ek_shader alphaMapShader = {};
    ek_shader solidColorShader = {};
    sg_image empty_image = {0};

    // Current drawing state
    sg_image image = {0};
    ek_shader program = {};
    Matrix3x2f matrix{};
    Rect2f uv{0.0f, 0.0f, 1.0f, 1.0f};
    ColorMod32 color{};
    Rect2f scissors{};

    // Current pass state
    bool active = false;
    sg_image renderTarget = {0};
    sg_image renderDepthStencil = {0};

    //// Offscreen rendering
    // framebuffer target could be also render target or null as default
    sg_image framebufferColor = {0};
    sg_image framebufferDepthStencil = {0};

    Matrix4f mvp{};

    // Stacks for save-restore states
    Array<Matrix3x2f> matrixStack{};
    Array<ColorMod32> colorStack{};
    Array<Rect2f> scissorsStack{};
    Array<ek_shader> programStack{};
    Array<sg_image> image_stack{};
    Array<Rect2f> texCoordStack{};

    // Checking what states could be potentially changed
    enum CheckFlags : uint8_t {
        Check_Scissors = 1,
        Check_Shader = 4,
        CHECK_IMAGE = 8
    };
    uint8_t checkFlags = 0;

    // Batch state tracking
    BatchState curr{};
    BatchState next{};
    bool stateChanged = true;

    // Stats during frame
    FrameStats stats;

    // Index data buffers
    BufferChain indexBuffers_;
    uint16_t* indexData_ = nullptr;
    uint16_t* indexDataPos_ = nullptr;
    uint16_t* indexDataNext_ = nullptr;
    uint32_t indicesCount_ = 0;

    // Vertex data buffers
    BufferChain vertexBuffers_;
    Vertex2D* vertexData_ = nullptr;
    Vertex2D* vertexDataPos_ = nullptr;
    Vertex2D* vertexDataNext_ = nullptr;
    uint32_t verticesCount_ = 0;
    uint16_t baseVertex_ = 0;

    // Batch rendering
    sg_pipeline selectedPipeline{};
    sg_bindings bind{};
    Hash<sg_pipeline> pipelines{};

    void createDefaultResources();
};

extern Context& state;

void beginNewFrame();

void begin(Rect2f viewport, const Matrix3x2f& view = Matrix3x2f{}, sg_image renderTarget = {0},
           sg_image depthStencilTarget = {0});

void end();

void endFrame();

void write_index(uint16_t index);

FrameStats getDrawStats();

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

void draw_indexed_triangles(const Array<Vec2f>& positions,
                            const Array<abgr32_t>& colors,
                            const Array<uint16_t>& indices,
                            Vec2f offset, Vec2f scale);

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


void initialize();

void shutdown();

}
