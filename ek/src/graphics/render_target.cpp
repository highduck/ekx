#include <ek/app/app.hpp>
#include "render_target.hpp"
#include "graphics.hpp"
#include "gl_debug.hpp"

namespace ek {

render_target_t::render_target_t(uint32_t width, uint32_t height, texture_type type) :
        width_{width},
        height_{height},
        texture_{new texture_t()} {

    texture_->reset(width, height, type);

    glGenFramebuffers(1, &frame_buffer_handle_);
    gl_check_error();

    if (type != texture_type::color32) {
        /** render buffers **/
        glGenRenderbuffers(1, &render_buffer_handle_);
        gl_check_error();

        // create render buffer and bind 16-bit depth buffer
        glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_handle_);
        GLenum render_buffer_format = 0;
        switch (type) {
            case texture_type::depth16:
            case texture_type::depth24:
                render_buffer_format = GL_DEPTH_COMPONENT16;
                break;
            case texture_type::color32:
                render_buffer_format = GL_RGBA8;
                break;
        }
        glRenderbufferStorage(GL_RENDERBUFFER, render_buffer_format, width, height);
        gl_check_error();
        //////
    }

    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_handle_);
    gl_check_error();

    GLenum attachment = GL_COLOR_ATTACHMENT0;
    if (type == texture_type::depth16 || type == texture_type::depth24) {
        attachment = GL_DEPTH_ATTACHMENT;
    }

    if (render_buffer_handle_ != 0) {
        // attach render buffer as depth buffer
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, render_buffer_handle_);
        gl_check_error();
        /////
    }

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture_->handle_, 0);
    gl_check_error();

    if (type == texture_type::depth16 || type == texture_type::depth24) {
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        gl_check_error();
    }

    GLenum framebuffer_status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (framebuffer_status != GL_FRAMEBUFFER_COMPLETE) {
        switch (framebuffer_status) {
//            case GL_FRAMEBUFFER_UNDEFINED_EXT:
//                assert(false);
//                break;
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
                assert(false);
                break;
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
                assert(false);
                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
//                assert(false);
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
//                assert(false);
//                break;
            case GL_FRAMEBUFFER_UNSUPPORTED:
                assert(false);
                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
//                assert(false);
//                break;
//            case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
//                assert(false);
//                break;
            default:
                assert(false);
                break;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, g_app.primary_frame_buffer);
    gl_check_error();
}

render_target_t::~render_target_t() {
    glDeleteFramebuffers(1, &frame_buffer_handle_);
    gl_check_error();

    if (render_buffer_handle_ != 0) {
        glDeleteRenderbuffers(1, &render_buffer_handle_);
        gl_check_error();
    }

    delete texture_;
}

void render_target_t::set() {
    glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_handle_);
    gl_check_error();
}

void render_target_t::unset() {
    glBindFramebuffer(GL_FRAMEBUFFER, g_app.primary_frame_buffer);
    gl_check_error();
}

void render_target_t::clear() {
    set();

    switch (texture_->type()) {
        case texture_type::color32:
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            gl_check_error();
            break;
        case texture_type::depth16:
        case texture_type::depth24:
            glClearDepth(1.0f);
            glClear(GL_DEPTH_BUFFER_BIT);
            gl_check_error();
            break;
    }

    unset();
}

void render_target_t::reset(uint32_t width, uint32_t height) {
    width_ = width;
    height_ = height;
    texture_->reset(width, height);
}

}
