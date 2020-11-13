#include "graphics.hpp"
#include <ek/util/logger.hpp>
#include <ek/app/app.hpp>
#include "gl_def.hpp"
#include "blending.hpp"
#include "gl_debug.hpp"

namespace ek::graphics {

using app::g_app;

static GraphicsContextType contextType = GraphicsContextType::OpenGL;

static int currentFrameBufferWidth = 1;
static int currentFrameBufferHeight = 1;

void init() {
    gl::skip_errors();

    const uint8_t* version = glGetString(GL_VERSION);
    gl::check_error();

    if (version) {
        EK_INFO << "OpenGL version: " << version;

        if (memcmp(version, "OpenGL ES ", 10) == 0) {
            const auto majorVersion = version[10];
            if (majorVersion == '2') {
                contextType = GraphicsContextType::OpenGL_ES_2;
            } else {
                contextType = GraphicsContextType::OpenGL_ES_3;
            }
        }
    }

    EK_INFO << "GLSL version: " << glGetString(GL_SHADING_LANGUAGE_VERSION);
    gl::check_error();

    glDepthMask(GL_FALSE);
    gl::check_error();
    glEnable(GL_BLEND);
    gl::check_error();
    glDisable(GL_DEPTH_TEST);
    gl::check_error();
    glDisable(GL_STENCIL_TEST);
    gl::check_error();
    glDisable(GL_DITHER);
    gl::check_error();
    glDisable(GL_CULL_FACE);
    gl::check_error();

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
    gl::check_error();
}

GraphicsContextType getContextType() {
    return contextType;
}

void begin() {
    gl::skip_errors();
}

void clear(float r, float g, float b, float a) {
    glClearColor(r, g, b, a);
    glClear(GL_COLOR_BUFFER_BIT);
    gl::check_error();
}

void viewport(int x, int y, int width, int height) {
    glViewport(x, y, width, height);
    gl::check_error();
    currentFrameBufferWidth = width;
    currentFrameBufferHeight = height;
}

void viewport() {
    viewport(0, 0,
             static_cast<int>(g_app.drawable_size.x),
             static_cast<int>(g_app.drawable_size.y));
}

void set_blend_mode(const blend_mode& blending) {
    glBlendFunc(static_cast<GLenum>(blending.source),
                static_cast<GLenum>(blending.dest));
    gl::check_error();
}

void set_scissors(int x, int y, int width, int height) {
    glEnable(GL_SCISSOR_TEST);
    gl::check_error();

    const int buffer_height = currentFrameBufferHeight;
    glScissor(x, buffer_height - y - height, width, height);
    gl::check_error();
}

void set_scissors() {
    glDisable(GL_SCISSOR_TEST);
    gl::check_error();
}

void get_pixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* out_buffer) {
    glReadPixels(x, currentFrameBufferHeight - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE, out_buffer);
    gl::check_error();
}

void draw_triangles(uint32_t indices_count) {
    glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_SHORT, nullptr);
    gl::check_error();
}

}
