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

class BufferChain;

struct BatchState {
    sg_shader shader{0};
    sg_image texture{0};
    Rect2i scissors{};
    BlendMode blend = BlendMode::PremultipliedAlpha;
    uint8_t shaderTexturesCount = 0;

    bool operator==(const BatchState& a) const {
        return (blend == a.blend && shader.id == a.shader.id && texture.id == a.texture.id);
    }

    bool operator!=(const BatchState& a) const {
        return (blend != a.blend || shader.id != a.shader.id || texture.id != a.texture.id);
    }
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

    Context& save_texture_coords();

    Context& setTextureCoords(float u0, float v0, float du, float dv);

    Context& setTextureCoords(const Rect2f& uv_rect);

    Context& restore_texture_coords();

    Context& saveTexture();

    Context& setEmptyTexture();

    Context& setTexture(const graphics::Texture* texture_);

    Context& setTextureRegion(const graphics::Texture* texture_ = nullptr,
                              const Rect2f& region = Rect2f{0.0f, 0.0f, 1.0f, 1.0f});

    Context& restoreTexture();

    Context& pushProgram(const char* id);

    Context& setProgram(const graphics::Shader* program_);

    Context& saveProgram();

    Context& restoreProgram();

    // do extra checking and clear states stack
    void finish();

    void setNextScissors(Rect2i rc);

    void setNextBlending(BlendMode blending);

    void setNextTexture(sg_image nextTexture);

    void setNextShader(sg_shader shader, uint8_t numTextures);

    void applyNextState();

public:

    void drawUserBatch(sg_pipeline pip, uint32_t numTextures);

    void drawBatch();

    void allocTriangles(uint32_t vertex_count, uint32_t index_count);

    [[nodiscard]]
    uint32_t getUsedMemory() const;

    sg_pipeline getPipeline(sg_shader shader, bool useRenderTarget, bool depthStencilPass);

public:

    constexpr static int MaxIndex = 0xFFFFF;
    constexpr static int MaxVertex = 0xFFFF;

    // Default resources
    graphics::Shader* defaultShader = nullptr;
    graphics::Shader* alphaMapShader = nullptr;
    graphics::Shader* solidColorShader = nullptr;
    graphics::Texture* emptyTexture = nullptr;

    // Current drawing state
    const graphics::Texture* texture = nullptr;
    const graphics::Shader* program = nullptr;
    Matrix3x2f matrix{};
    Rect2f uv{0.0f, 0.0f, 1.0f, 1.0f};
    ColorMod32 color{};
    Rect2f scissors{};

    // Current pass state
    bool active = false;
    const graphics::Texture* renderTarget = nullptr;
    const graphics::Texture* renderDepthStencil = nullptr;

    //// Offscreen rendering
    // framebuffer target could be also render target or null as default
    const graphics::Texture* framebufferColor = nullptr;
    const graphics::Texture* framebufferDepthStencil = nullptr;

    Matrix4f mvp{};

    // Stacks for save-restore states
    Array<Matrix3x2f> matrixStack{};
    Array<ColorMod32> colorStack{};
    Array<Rect2f> scissorsStack{};
    Array<const graphics::Shader*> programStack{};
    Array<const graphics::Texture*> textureStack{};
    Array<Rect2f> texCoordStack{};

    // Checking what states could be potentially changed
    enum CheckFlags : uint8_t {
        Check_Scissors = 1,
        Check_Shader = 4,
        Check_Texture = 8
    };
    uint8_t checkFlags = 0;

    // Batch state tracking
    BatchState curr{};
    BatchState next{};
    bool stateChanged = true;

    // Stats during frame
    FrameStats stats;

    // Index data buffers
    BufferChain* indexBuffers_ = nullptr;
    uint16_t* indexData_ = nullptr;
    uint16_t* indexDataPos_ = nullptr;
    uint16_t* indexDataNext_ = nullptr;
    uint32_t indicesCount_ = 0;

    // Vertex data buffers
    BufferChain* vertexBuffers_ = nullptr;
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

void begin(Rect2f viewport, const Matrix3x2f& view = Matrix3x2f{}, const graphics::Texture* renderTarget = nullptr, const graphics::Texture* depthStencilTarget = nullptr);

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
