#pragma once

#if defined(NDEBUG)
#define GL_CHECK(call) call;
#else
#define GL_CHECK(call) call;::ek::graphics::gl::check_error(#call)
#endif

namespace ek::graphics::gl {

void set_debugging(bool enabled);

void check_error(const char* cmd = nullptr);

void skip_errors();

}

