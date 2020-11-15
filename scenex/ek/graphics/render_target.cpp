#include <ek/app/app.hpp>
#include "render_target.hpp"
#include "graphics.hpp"
#include "gl_debug.hpp"

namespace ek::graphics {

using ek::app::g_app;

render_target_t::render_target_t(uint32_t width, uint32_t height, texture_type type) :
        width_{width},
        height_{height},
        texture_{new texture_t()} {

    texture_->reset(width, height, type);

    GL_CHECK(glGenFramebuffers(1, &frame_buffer_handle_));

    if (type != texture_type::color32) {
        /** render buffers **/
        GL_CHECK(glGenRenderbuffers(1, &render_buffer_handle_));

        // create render buffer and bind 16-bit depth buffer
        GL_CHECK(glBindRenderbuffer(GL_RENDERBUFFER, render_buffer_handle_));
        GLenum render_buffer_format = 0;
        switch (type) {
            case texture_type::depth16:
            case texture_type::depth24:
                render_buffer_format = GL_DEPTH_COMPONENT16;
                break;
            case texture_type::color32:
                render_buffer_format = GL_RGBA8;
                break;
            case texture_type::alpha8:
                render_buffer_format = GL_R8;
                break;
        }
        GL_CHECK(glRenderbufferStorage(GL_RENDERBUFFER, render_buffer_format, width, height));
        //////
    }

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_handle_));

    GLenum attachment = GL_COLOR_ATTACHMENT0;
    if (type == texture_type::depth16 || type == texture_type::depth24) {
        attachment = GL_DEPTH_ATTACHMENT;
    }

    if (render_buffer_handle_ != 0) {
        // attach render buffer as depth buffer
        GL_CHECK(glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, render_buffer_handle_));
        /////
    }

    GL_CHECK(glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture_->handle_, 0));

    if (type == texture_type::depth16 || type == texture_type::depth24) {
        GL_CHECK(glDrawBuffer(GL_NONE));
        GL_CHECK(glReadBuffer(GL_NONE));
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

    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, g_app.primary_frame_buffer));
}

render_target_t::~render_target_t() {
    GL_CHECK(glDeleteFramebuffers(1, &frame_buffer_handle_));

    if (render_buffer_handle_ != 0) {
        GL_CHECK(glDeleteRenderbuffers(1, &render_buffer_handle_));
    }

    delete texture_;
}

void render_target_t::set() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, frame_buffer_handle_));
}

void render_target_t::unset() {
    GL_CHECK(glBindFramebuffer(GL_FRAMEBUFFER, g_app.primary_frame_buffer));
}

void render_target_t::clear() {
    set();

    switch (texture_->type()) {
        case texture_type::color32:
        case texture_type::alpha8: {
            GL_CHECK(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
            GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
        }
            break;
        case texture_type::depth16:
        case texture_type::depth24: {
            GL_CHECK(glClearDepth(1.0f));
            GL_CHECK(glClear(GL_DEPTH_BUFFER_BIT));
        }
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
