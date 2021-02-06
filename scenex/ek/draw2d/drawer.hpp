#pragma once

#include <vector>
#include <ek/util/common_macro.hpp>
#include <ek/util/Res.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/math/mat4x4.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/circle.hpp>
#include <ek/math/box.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/graphics/Helpers.hpp>

namespace ek::draw2d {

struct FrameStats {
    uint32_t triangles = 0u;
    uint32_t drawCalls = 0u;
    float fillArea = 0.0f;
};

struct Vertex2D {
    float2 position;
    float2 uv;
    abgr32_t cm;
    abgr32_t co;
};

enum class BlendMode : uint8_t {
    PremultipliedAlpha = 0
};

class BufferChain;

struct BatchState {
    sg_shader shader{0};
    sg_image texture{0};
    rect_i scissors{};
    BlendMode blend = BlendMode::PremultipliedAlpha;
    uint8_t shaderTexturesCount = 0;

    bool operator==(const BatchState& a) const {
        return (blend == a.blend && shader.id == a.shader.id && texture.id == a.texture.id);
    }

    bool operator!=(const BatchState& a) const {
        return (blend != a.blend || shader.id != a.shader.id || texture.id != a.texture.id);
    }
};

struct Context : private disable_copy_assign_t {

//    Context();
//
//    ~Context();

    void initialize();

    void shutdown();

    /** Scissors **/

    void pushClipRect(const rect_f& rc);

    Context& saveScissors();

    void setScissors(const rect_f& rc);

    Context& popClipRect();

    /** Matrix Transform **/

    Context& save_matrix();

    Context& save_transform();

    Context& restore_transform();

    Context& transform_pivot(float2 position, float rotation, float2 scale, float2 pivot) {
        matrix.translate(position.x + pivot.x, position.y + pivot.y)
                .scale(scale.x, scale.y)
                .rotate(rotation)
                .translate(-pivot.x, -pivot.y);
        return *this;
    }

    Context& translate(float tx, float ty);

    Context& translate(const float2& v);

    Context& scale(float sx, float sy);

    Context& scale(const float2& v);

    Context& rotate(float radians);

    Context& concat(const matrix_2d& r);

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

    Context& set_texture_coords(float u0, float v0, float du, float dv);

    Context& set_texture_coords(const rect_f& uv_rect);

    Context& restore_texture_coords();

    Context& save_texture();

    Context& set_empty_texture();

    Context& set_texture(const graphics::Texture* texture);

    Context& set_texture_region(const graphics::Texture* texture = nullptr,
                                const rect_f& region = rect_f::zero_one);

    Context& restore_texture();

    Context& pushProgram(const char* id);

    Context& setProgram(const graphics::Shader* program_);

    Context& saveProgram();

    Context& restoreProgram();

    // do extra checking and clear states stack
    void finish();

    void setNextScissors(rect_i rc);

    void setNextBlending(BlendMode blending);

    void setNextTexture(sg_image texture);

    void setNextShader(sg_shader shader, uint8_t numTextures);

    void applyNextState();

public:

    void drawBatch();

    void allocTriangles(uint32_t vertex_count, uint32_t index_count);

    [[nodiscard]]
    uint32_t getUsedMemory() const;

    sg_pipeline getPipeline(sg_shader shader, bool useRenderTarget);

public:

    constexpr static int MaxIndex = 0xFFFFF;
    constexpr static int MaxVertex = 0xFFFF;

    // Default resources
    const graphics::Shader* defaultShader = nullptr;
    const graphics::Shader* alphaMapShader = nullptr;
    const graphics::Shader* solidColorShader = nullptr;
    const graphics::Texture* emptyTexture = nullptr;

    // Current drawing state
    const graphics::Texture* texture = nullptr;
    const graphics::Shader* program = nullptr;
    matrix_2d matrix{};
    rect_f uv{0.0f, 0.0f, 1.0f, 1.0f};
    ColorMod32 color{};
    rect_f scissors{};

    // Current pass state
    bool active = false;
    const graphics::Texture* renderTarget = nullptr;
    mat4f mvp{};

    // Stacks for save-restore states
    std::vector<matrix_2d> matrixStack;
    std::vector<ColorMod32> colorStack;
    std::vector<rect_f> scissorsStack;
    std::vector<const graphics::Shader*> programStack;
    std::vector<const graphics::Texture*> textureStack;
    std::vector<rect_f> texCoordStack;

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
    std::unordered_map<uint64_t, sg_pipeline> pipelines{};

    bool initialized_ = false;
};

extern Context state;

void beginNewFrame();

void begin(rect_i viewport, const matrix_2d& view = matrix_2d{}, const graphics::Texture* renderTarget = nullptr);

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

void write_indices(const uint16_t* source, uint16_t count, uint16_t base_vertex = 0);

void draw_indexed_triangles(const std::vector<float2>& positions,
                            const std::vector<abgr32_t>& colors,
                            const std::vector<uint16_t>& indices,
                            float2 offset, float2 scale);

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