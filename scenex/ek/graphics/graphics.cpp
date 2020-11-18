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

    GL_CHECK(glDepthMask(GL_FALSE));
    GL_CHECK(glEnable(GL_BLEND));
    GL_CHECK(glDisable(GL_DEPTH_TEST));
    GL_CHECK(glDisable(GL_STENCIL_TEST));
    GL_CHECK(glDisable(GL_DITHER));
    GL_CHECK(glDisable(GL_CULL_FACE));

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

    GL_CHECK(glFinish());
}

GraphicsContextType getContextType() {
    return contextType;
}

void begin() {
    gl::skip_errors();
}

void clear(float r, float g, float b, float a) {
    GL_CHECK(glClearColor(r, g, b, a));
    GL_CHECK(glClear(GL_COLOR_BUFFER_BIT));
}

void viewport(int x, int y, int width, int height) {
    GL_CHECK(glViewport(x, y, width, height));
    currentFrameBufferWidth = width;
    currentFrameBufferHeight = height;
}

void viewport() {
    auto w = static_cast<int>(g_app.drawable_size.x);
    auto h = static_cast<int>(g_app.drawable_size.y);
    viewport(0, 0, w, h);
}

void set_blend_mode(const blend_mode& blending) {
    auto src = static_cast<GLenum>(blending.source);
    auto dst = static_cast<GLenum>(blending.dest);
    GL_CHECK(glBlendFunc(src, dst));
}

void set_scissors(int x, int y, int width, int height) {
    const int buffer_height = currentFrameBufferHeight;
    GL_CHECK(glEnable(GL_SCISSOR_TEST));
    if (width < 0) width = 0;
    if (height < 0) height = 0;
    GL_CHECK(glScissor(x, buffer_height - y - height, width, height));
}

void set_scissors() {
    GL_CHECK(glDisable(GL_SCISSOR_TEST));
}

void get_pixels(uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t* out_buffer) {
    GL_CHECK(glReadPixels(x, currentFrameBufferHeight - y - height, width, height, GL_RGBA, GL_UNSIGNED_BYTE,
                          out_buffer));
}

void draw_triangles(uint32_t indices_count) {
    GL_CHECK(glDrawElements(GL_TRIANGLES, indices_count, GL_UNSIGNED_SHORT, nullptr));
}

}
