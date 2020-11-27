#pragma once

#include <ek/graphics/vertex_decl.hpp>
#include <ek/graphics/graphics.hpp>

#include "batch_state_manager.hpp"

namespace ek {

struct FrameStats {
    uint32_t triangles = 0u;
    uint32_t drawCalls = 0u;
    float fillArea = 0.0f;
};

class BufferChain;

class Batcher : private disable_copy_assign_t {
public:
    constexpr static int max_indices_limit = 0x100000;
    constexpr static int max_vertices_limit = 0xFFFF;

    batch_state_context states;
    FrameStats stats;

public:

    Batcher();

    ~Batcher();

    void begin();

    void draw();

    // force to set current state to device immediately
    void invalidate_force();

    void draw_mesh(const graphics::buffer_t& vb, const graphics::buffer_t& ib, int32_t indices_count);

    void flush();

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
};

}