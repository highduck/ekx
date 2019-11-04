#include "graphics.hpp"

#include <platform/Window.h>
#include <ek/logger.hpp>
#include <ek/utility/common_macro.hpp>
#include "gl_def.hpp"
#include "blend_mode.hpp"
#include "gl_debug.hpp"

namespace ek {

graphics_t::graphics_t() {

    assert_created_once<graphics_t>();

    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_DITHER);
    glDisable(GL_CULL_FACE);
    gl_check_error();

    // TODO:
//		if (true) {
//			GL.enable(GL.MULTISAMPLE);
//		}
//		else {
//			GL.disable(GL.MULTISAMPLE);
//		}
#ifdef __EMSCRIPTEN__
    //    glPixelStorei(GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL, 1);
    //    glCheckError();
#endif

    glFinish();
    gl_check_error();

    EK_INFO << "OpenGL version: " << glGetString(GL_VERSION);
    EK_INFO << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
}

graphics_t::~graphics_t() = default;

graphics_t::api_type graphics_t::api() const {
    return api_type::gl;
}

void graphics_t::begin() {
    gl_skip_errors();
}

void graphics_t::clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    gl_check_error();
}

void graphics_t::viewport(int x, int y, int wight, int height) {
    glViewport(x, y, wight, height);
    gl_check_error();
}

void graphics_t::viewport() {
    glViewport(0, 0, gWindow.backBufferSize.width, gWindow.backBufferSize.height);
    gl_check_error();
}

void graphics_t::set_blend_mode(const blend_mode& blend_mode) {
    glBlendFunc(static_cast<GLenum>(blend_mode.source),
                static_cast<GLenum>(blend_mode.dest));
    gl_check_error();
}

void graphics_t::set_scissors(int x, int y, int width, int height) {
    glEnable(GL_SCISSOR_TEST);
    gl_check_error();

    const int buffer_height = gWindow.backBufferSize.height;
    glScissor(x, buffer_height - y - height, width, height);
    gl_check_error();
}

void graphics_t::set_scissors() {
    glDisable(GL_SCISSOR_TEST);
    gl_check_error();
}

void graphics_t::get_pixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* out_buffer) {
    const int buffer_height = gWindow.backBufferSize.height;

    glReadPixels(x, buffer_height - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE, out_buffer);
    gl_check_error();
}

void graphics_t::draw_triangles(uint32_t indices_count) {
    glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_SHORT, nullptr);
    gl_check_error();
}

}
