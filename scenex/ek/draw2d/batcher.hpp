#pragma once

#include <ek/graphics/graphics.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/mat4x4.hpp>
#include <ek/math/box.hpp>

namespace ek::draw2d {

struct Vertex2D {
    float2 position;
    float2 uv;
    abgr32_t cm;
    abgr32_t co;
};

struct FrameStats {
    uint32_t triangles = 0u;
    uint32_t drawCalls = 0u;
    float fillArea = 0.0f;
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

class Batcher : private disable_copy_assign_t {
public:
    constexpr static int max_indices_limit = 0x100000;
    constexpr static int max_vertices_limit = 0xFFFF;

    BatchState curr{};
    BatchState next{};
    bool stateChanged = true;

    const graphics::Texture* renderTarget = nullptr;
    mat4f mvp{};
    FrameStats stats;

    void beginNewFrame();

    void setScissors(rect_i rc);

    void setBlendMode(BlendMode blending);

    void setTexture(sg_image texture);

    void setProgram(sg_shader shader, uint8_t numTextures);

    void clear();

    void setNextState();

public:

    Batcher();

    ~Batcher();

    void beginPass();

    void draw();

    void endPass();

    void completeFrame();

    void alloc_triangles(int vertex_count, int index_count);

    [[nodiscard]]
    inline uint8_t* vertex_memory_ptr() const {
        return vertex_memory_ + vertex_pointer_;
    }

    [[nodiscard]]
    inline uint16_t* index_memory_ptr() const {
        return index_memory_ + index_pointer_;
    }

    [[nodiscard]]
    inline uint16_t get_vertex_index(uint16_t base_vertex = 0u) const {
        return base_vertex_ + base_vertex;
    }

    [[nodiscard]]
    uint32_t getUsedMemory() const;

private:

    void init_memory(uint32_t vertex_max_size, uint32_t vertices_limit, uint32_t indices_limit);

    BufferChain* vertexBuffers_ = nullptr;
    BufferChain* indexBuffers_ = nullptr;
    uint32_t vertex_max_size_ = 0;
    uint32_t vertex_index_max_ = 0;
    uint32_t vertices_count_ = 0;
    uint32_t indices_count_ = 0;
    uint32_t next_vertex_pointer_ = 0;
    uint32_t next_index_pointer_ = 0;

    uint8_t* vertex_memory_ = nullptr;
    uint16_t* index_memory_ = nullptr;
    uint32_t vertex_pointer_ = 0;
    uint32_t index_pointer_ = 0;

    uint32_t vertex_size_ = 0;
    uint16_t base_vertex_ = 0;

    sg_pipeline selectedPipeline{};
    sg_bindings bind{};
//    mat4f mvp;
};

}