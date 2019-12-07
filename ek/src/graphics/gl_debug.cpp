#include "gl_debug.hpp"

#include "gl_def.hpp"
#include <ek/util/logger.hpp>
#include <cstdlib>

namespace ek {

#if defined(__EMSCRIPTEN__)

#define EK_NO_GL_CHECKS

#endif

#if defined(NDEBUG)
static bool gl_debugging_enabled_ = false;
#else
static bool gl_debugging_enabled_ = true;
#endif

void gl_set_debugging(bool enabled) {
    gl_debugging_enabled_ = enabled;
}

void gl_skip_errors() {
#ifndef EK_NO_GL_CHECKS
    if (gl_debugging_enabled_) {
        int totalErrors = 0;
        while (glGetError() != GL_NO_ERROR) {
            ++totalErrors;
            if (totalErrors >= 50) {
                abort();
            }
        }
    }
#endif
}

void gl_check_error() {
#ifndef EK_NO_GL_CHECKS
    if (gl_debugging_enabled_) {
        auto err = glGetError();
        if (err != GL_NO_ERROR) {
            const char* error = "";

            switch (err) {
                case GL_INVALID_OPERATION:
                    error = "INVALID_OPERATION";
                    break;
                case GL_INVALID_ENUM:
                    error = "INVALID_ENUM";
                    break;
                case GL_INVALID_VALUE:
                    error = "INVALID_VALUE";
                    break;
                case GL_OUT_OF_MEMORY:
                    error = "OUT_OF_MEMORY";
                    break;
                case GL_INVALID_FRAMEBUFFER_OPERATION:
                    error = "INVALID_FRAMEBUFFER_OPERATION";
                    break;
                default:
                    break;
            }
            EK_WARN("glGetError %i ( %s )", err, error);
        }
        if (err != GL_NO_ERROR) {
            abort();
        }
    }
#endif
}

}
