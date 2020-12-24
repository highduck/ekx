#include "batcher.hpp"

#include <ek/graphics/Helpers.hpp>
#include <vector>
#include <unordered_map>

namespace ek::draw2d {

using namespace ek::graphics;

void Batcher::setScissors(rect_i rc) {
    if (rc != curr.scissors) {
        stateChanged = true;
    }
    next.scissors = rc;
}

void Batcher::setBlendMode(BlendMode blending) {
    if (curr.blend != blending) {
        stateChanged = true;
    }
    next.blend = blending;
}

void Batcher::setTexture(sg_image texture) {
    if (curr.texture.id != texture.id) {
        stateChanged = true;
    }
    next.texture = texture;
}

void Batcher::setProgram(sg_shader shader, uint8_t numTextures) {
    if (curr.shader.id != shader.id) {
        stateChanged = true;
    }
    next.shader = shader;
    next.shaderTexturesCount = numTextures;
}

void Batcher::setNextState() {
    if (stateChanged) {
        curr = next;
        stateChanged = false;
    }
}

static std::unordered_map<uint64_t, sg_pipeline> pipelineCache{};

sg_pipeline createPipeline(sg_shader shader, bool renderTarget) {
    sg_pipeline_desc pip_desc{};
    pip_desc.layout.buffers[0].stride = sizeof(Vertex2D);
    pip_desc.layout.attrs[0].offset = offsetof(Vertex2D, position);
    pip_desc.layout.attrs[0].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[1].offset = offsetof(Vertex2D, uv);
    pip_desc.layout.attrs[1].format = SG_VERTEXFORMAT_FLOAT2;
    pip_desc.layout.attrs[2].offset = offsetof(Vertex2D, cm);
    pip_desc.layout.attrs[2].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.layout.attrs[3].offset = offsetof(Vertex2D, co);
    pip_desc.layout.attrs[3].format = SG_VERTEXFORMAT_UBYTE4N;
    pip_desc.shader = shader;
    pip_desc.index_type = SG_INDEXTYPE_UINT16;
    pip_desc.blend.enabled = true;
    pip_desc.blend.src_factor_rgb = SG_BLENDFACTOR_ONE;
    pip_desc.blend.dst_factor_rgb = SG_BLENDFACTOR_ONE_MINUS_SRC_ALPHA;
    pip_desc.blend.color_write_mask = SG_COLORMASK_RGB;
    pip_desc.primitive_type = SG_PRIMITIVETYPE_TRIANGLES;
    if (renderTarget) {
        pip_desc.blend.color_format = SG_PIXELFORMAT_RGBA8;
        pip_desc.blend.depth_format = SG_PIXELFORMAT_NONE;
    }
    pip_desc.depth_stencil.depth_write_enabled = false;
    pip_desc.depth_stencil.stencil_enabled = false;
    pip_desc.rasterizer.sample_count = 0;
    pip_desc.label = "draw2d-pipeline";
    return sg_make_pipeline(pip_desc);
}

sg_pipeline getPipeline(sg_shader shader, bool renderTarget) {
    uint64_t key{shader.id};
    if (renderTarget) {
        key = key | (1ull << 32ull);
    }
    auto it = pipelineCache.find(key);
    if (it == pipelineCache.end()) {
        pipelineCache[key] = createPipeline(shader, renderTarget);
    }
    return pipelineCache[key];
}

float getTriangleArea(const Vertex2D* vertices, const uint16_t* indices, int count) {
    float sum = 0.0f;
    for (int i = 0; i < count;) {
        float2 a = vertices[indices[i++]].position;
        float2 b = vertices[indices[i++]].position;
        float2 c = vertices[indices[i++]].position;
        sum += (a.x * b.y + b.x * c.y + c.x * a.y - a.x * c.y - b.x * a.y - c.x * b.y) / 2.0f;
    }
    return sum;
}

class BufferChain {
public:
    explicit BufferChain(BufferType type) : type_{type} {
        // manual buffering
        // framesNum_ = 3; // triple-buffering
        buffersLimit_ = 0; // unlimited

        // orphaning
        framesNum_ = 1;
//        buffersLimit_ = 1;
        useOrphaning = true;

        if (type == BufferType::IndexBuffer) {
            maxSize = 0xFFFF * 3 * sizeof(uint16_t);
        } else {
            maxSize = 0xFFFF * sizeof(Vertex2D);
        }
    }

    Buffer* nextBuffer() {
        Buffer* buf;
        if (pos >= buffers_.size()) {
            buf = new Buffer(type_, Usage::Dynamic, maxSize);
            //buf->useDataOrphaning = useOrphaning;
            buffers_.push_back(buf);
        } else {
            buf = buffers_[pos];
        }
        ++pos;
        if (buffersLimit_ > 0 && pos >= buffersLimit_) {
            pos = 0;
        }
        return buf;
    }

    [[nodiscard]]
    uint32_t getUsedMemory() const {
        uint32_t mem = 0u;
        for (const auto* buffer : buffers_) {
            mem += buffer->getSize();
        }
        return mem;
    }

    void nextFrame() {
        ++frame;
        if (frame >= framesNum_) {
            frame = 0;
            pos = 0;
        }
    }

    void reset() {
        for (auto* b : buffers_) {
            delete b;
        }
        buffers_.resize(0);
    }

    ~BufferChain() {
        reset();
    }

private:
    BufferType type_;
    std::vector<Buffer*> buffers_;
    int pos = 0;
    int frame = 0;
    int buffersLimit_;
    int framesNum_;
    bool useOrphaning = true;
    uint32_t maxSize;
};

Batcher::Batcher() {
    vertexBuffers_ = new BufferChain(BufferType::VertexBuffer);
    indexBuffers_ = new BufferChain(BufferType::IndexBuffer);
    init_memory(sizeof(Vertex2D), max_vertices_limit, max_indices_limit);
}

Batcher::~Batcher() {
    delete vertexBuffers_;
    delete indexBuffers_;
    delete[] vertex_memory_;
    delete[] index_memory_;
}

void Batcher::init_memory(uint32_t vertex_max_size, uint32_t vertices_limit, uint32_t indices_limit) {
    assert(vertex_max_size > 0 && vertex_max_size % 4 == 0);
    assert(vertices_limit > 0 && vertices_limit <= max_vertices_limit);
    assert(indices_limit > 0 && indices_limit <= max_indices_limit);

    vertex_size_ = vertex_max_size;
    vertex_max_size_ = vertex_max_size;
    vertex_index_max_ = vertices_limit - 1;

    next_index_pointer_ = 0;

    vertex_memory_ = new uint8_t[vertices_limit * vertex_max_size];
    index_memory_ = new uint16_t[indices_limit];
}

void Batcher::beginPass() {
    curr = {};
    next = {};
    stateChanged = true;
    selectedPipeline.id = SG_INVALID_ID;
}

void Batcher::draw() {
    if (indices_count_ == 0) {
        return;
    }

    auto* vb = vertexBuffers_->nextBuffer();
    auto* ib = indexBuffers_->nextBuffer();
    vb->update(vertex_memory_, next_vertex_pointer_);
    ib->update(index_memory_, next_index_pointer_ << 1u);

    auto pip = getPipeline(curr.shader, renderTarget != nullptr);
    if (pip.id != selectedPipeline.id) {
        selectedPipeline = pip;
        sg_apply_pipeline(pip);
        sg_apply_uniforms(SG_SHADERSTAGE_VS, 0, &mvp, sizeof(mvp));
    }
    bind.vertex_buffers[0] = vb->buffer;
    bind.index_buffer = ib->buffer;
    bind.fs_images[0].id = curr.shaderTexturesCount == 1 ? curr.texture.id : SG_INVALID_ID;
    sg_apply_bindings(bind);

    auto scissors = curr.scissors;
    sg_apply_scissor_rect(scissors.x, scissors.y, scissors.width, scissors.height, true);

    sg_draw(0, (int) indices_count_, 1);

#ifndef NDEBUG
    stats.fillArea += getTriangleArea(reinterpret_cast<const Vertex2D*>(vertex_memory_),
                                      static_cast<const uint16_t*>(index_memory_), next_index_pointer_ << 1u);
#endif
    stats.triangles += indices_count_ / 3;
    ++stats.drawCalls;

    // reset stream pointers
    next_index_pointer_ = 0;
    indices_count_ = 0;
    next_vertex_pointer_ = 0;
    vertices_count_ = 0;
}

void Batcher::alloc_triangles(int vertex_count, int index_count) {
    if (stateChanged || (vertices_count_ + vertex_count) > vertex_index_max_) {
        draw();
        setNextState();
    }

    base_vertex_ = vertices_count_;
    index_pointer_ = next_index_pointer_;
    next_index_pointer_ += index_count;
    indices_count_ += index_count;

    vertex_pointer_ = next_vertex_pointer_;
    next_vertex_pointer_ += vertex_count * vertex_size_;
    vertices_count_ += vertex_count;
}

uint32_t Batcher::getUsedMemory() const {
    return indexBuffers_->getUsedMemory() + vertexBuffers_->getUsedMemory();
}

void Batcher::endPass() {
    draw();
}

void Batcher::beginNewFrame() {
    stats = {};
}

void Batcher::completeFrame() {
    vertexBuffers_->nextFrame();
    indexBuffers_->nextFrame();
}

}