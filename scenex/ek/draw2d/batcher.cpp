#include "batcher.hpp"

#include <ek/graphics/buffer.hpp>
#include <ek/graphics/program.hpp>
#include <ek/graphics/gl_debug.hpp>
#include <vector>

namespace ek {

using namespace ek::graphics;

class BufferChain {
public:
    explicit BufferChain(buffer_type type) :
            type_{type} {
        // manual buffering
        // framesNum_ = 3; // triple-buffering
        // buffersLimit_ = 0; // unlimited

        // orphaning
        framesNum_ = 1;
        buffersLimit_ = 1;
        useOrphaning = true;
    }

    buffer_t* nextBuffer() {
        buffer_t* buf;
        if (pos >= buffers_.size()) {
            buf = new buffer_t(type_, buffer_usage::dynamic_buffer);
            buf->useDataOrphaning = useOrphaning;
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
    buffer_type type_;
    std::vector<buffer_t*> buffers_;
    int pos = 0;
    int frame = 0;
    int buffersLimit_;
    int framesNum_;
    bool useOrphaning = true;
};

Batcher::Batcher() {
    vertexBuffers_ = new BufferChain(buffer_type::vertex_buffer);
    indexBuffers_ = new BufferChain(buffer_type::index_buffer);
    init_memory(sizeof(vertex_2d), max_vertices_limit, max_indices_limit);
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

void Batcher::begin() {
    vertexBuffers_->nextFrame();
    indexBuffers_->nextFrame();
}

void Batcher::draw() {
    states.apply();

//        glGenVertexArrays(1, &vao);
//        glCheckError();
//        glBindVertexArray(vao);
//        glCheckError();

    auto* vb = vertexBuffers_->nextBuffer();
    auto* ib = indexBuffers_->nextBuffer();
    vb->upload(vertex_memory_, next_vertex_pointer_);
    ib->upload(index_memory_, next_index_pointer_ << 1u);

    states.curr.program->bind_attributes();
    states.curr.program->bind_image();

    graphics::draw_triangles(indices_count_);

    states.curr.program->unbind_attributes();

//        glBindVertexArray(0);
//        glCheckError();
//        glDeleteVertexArrays(1, &vao);
//        glCheckError();


    stats.triangles += indices_count_ / 3;
    ++stats.draw_calls;

    // reset stream pointers
    next_index_pointer_ = 0;
    indices_count_ = 0;
    next_vertex_pointer_ = 0;
    vertices_count_ = 0;
}

void Batcher::alloc_triangles(int vertex_count, int index_count) {
    if (states.anyChanged || (vertices_count_ + vertex_count) > vertex_index_max_) {
        flush();
        states.invalidate();
    }

    base_vertex_ = vertices_count_;
    index_pointer_ = next_index_pointer_;
    next_index_pointer_ += index_count;
    indices_count_ += index_count;

    vertex_pointer_ = next_vertex_pointer_;
    next_vertex_pointer_ += vertex_count * vertex_size_;
    vertices_count_ += vertex_count;
}

void Batcher::flush() {
    if (vertices_count_ > 0) {
        draw();
    }
}

void Batcher::invalidate_force() {
    if (states.anyChanged || true) {
        flush();
        states.invalidate();
    }
//
//    mesh.bind();

    states.apply();
}

void Batcher::draw_mesh(const graphics::buffer_t& vb,
                        const graphics::buffer_t& ib,
                        int32_t indices_count) {
    vb.bind();
    ib.bind();

    states.curr.program->bind_attributes();
    states.curr.program->bind_image();

    //program.enableVertexAttributes(vertices.vertexDeclaration);
    glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_SHORT, nullptr);
    graphics::gl::check_error();
    //_program.disableVertexAttributes(vertices.vertexDeclaration);
    states.curr.program->unbind_attributes();
}

uint32_t Batcher::getUsedMemory() const {
    return indexBuffers_->getUsedMemory() + vertexBuffers_->getUsedMemory();
}

}