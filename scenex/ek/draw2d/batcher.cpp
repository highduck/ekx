#include "batcher.hpp"

#include <ek/graphics/buffer.hpp>
#include <ek/graphics/program.hpp>
#include <ek/graphics/gl_debug.hpp>

namespace ek {

batcher_t::batcher_t() {
    using namespace ek::graphics;
    vertex_buffer_ = new buffer_t(buffer_type::vertex_buffer, buffer_usage::dynamic_buffer);
    index_buffer_ = new buffer_t(buffer_type::index_buffer, buffer_usage::dynamic_buffer);
    init_memory(sizeof(vertex_2d), max_vertices_limit, max_indices_limit);
}

batcher_t::~batcher_t() {
    delete vertex_buffer_;
    delete index_buffer_;
    delete[] vertex_memory_;
    delete[] index_memory_;
}

void batcher_t::init_memory(uint32_t vertex_max_size, uint32_t vertices_limit, uint32_t indices_limit) {
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

void batcher_t::begin() {

}

void batcher_t::draw() {
    states.apply();

//        glGenVertexArrays(1, &vao);
//        glCheckError();
//        glBindVertexArray(vao);
//        glCheckError();

    vertex_buffer_->upload(vertex_memory_, next_vertex_pointer_);
    index_buffer_->upload(index_memory_, next_index_pointer_ << 1u);

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

void batcher_t::alloc_triangles(int vertex_count, int index_count) {
    if (states.changed || (vertices_count_ + vertex_count) > vertex_index_max_) {
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

void batcher_t::flush() {
    if (vertices_count_ > 0) {
        draw();
    }
}

void batcher_t::temp_begin_mesh() {
    if (states.changed || true) {
        flush();
        states.invalidate();
    }
//
//    mesh.bind();

    states.apply();
}

void batcher_t::temp_draw_static_mesh(const graphics::buffer_t& vb,
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

}