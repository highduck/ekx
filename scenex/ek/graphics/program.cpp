#include "program.hpp"
#include "vertex_decl.hpp"
#include "gl_debug.hpp"
#include <ek/util/logger.hpp>

namespace ek::graphics {

using namespace program_uniforms;
using namespace program_attributes;

void print_shader_compile_error(GLuint handle, const char* user_data) {
    GLint log_length = 0;
    EK_ERROR << "Shader compilation failed! " << user_data;
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        char buf[256];
        glGetShaderInfoLog(handle, sizeof(buf) - 1, nullptr, (GLchar*) buf);
        EK_ERROR << buf;
    }
}

void print_program_compile_error(GLuint handle, const char* user_data) {
    GLint log_length = 0;
    EK_ERROR << "Program link failed! " << user_data;
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if (log_length > 0) {
        char buf[256];
        glGetProgramInfoLog(handle, sizeof(buf) - 1, nullptr, (GLchar*) buf);
        EK_ERROR << buf;
    }
}

bool check_shader(GLuint handle, const char* user_data) {
    GLint status = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    if ((GLboolean) status == GL_FALSE) {
        print_shader_compile_error(handle, user_data);
        return false;
    }
    return true;
}

bool check_program(GLuint handle, const char* user_data) {
    GLint status = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    if ((GLboolean) status == GL_FALSE) {
        print_program_compile_error(handle, user_data);
        return false;
    }
    return true;
}

program_t::program_t(const char* vs_code, const char* fs_code) {
    // could be used for `#version` pragma later
#if (TARGET_OS_IOS) || defined(__ANDROID__) || defined(__EMSCRIPTEN__)
    // 1.00 es2
    //const char* defines = "#version 100\n";
    const char* defines = "";
#else
    const char* defines = "#version 120\n";
#endif

    const char* vs[2] = {defines, vs_code};
    const char* fs[2] = {defines, fs_code};

    GLuint vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 2, vs, nullptr);
    glCompileShader(vertex_shader);
    if (!check_shader(vertex_shader, "vertex shader")) {
        glDeleteShader(vertex_shader);
        return;
    }

    GLuint fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 2, fs, nullptr);
    glCompileShader(fragment_shader);
    if (!check_shader(fragment_shader, "fragment shader")) {
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        return;
    }

    handle_ = glCreateProgram();
    glAttachShader(handle_, vertex_shader);
    glAttachShader(handle_, fragment_shader);
    glLinkProgram(handle_);
    if (!check_program(handle_, "shader program")) {
        glDeleteProgram(handle_);
        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);
        handle_ = 0;
        return;
    }

    glDetachShader(handle_, vertex_shader);
    glDetachShader(handle_, fragment_shader);

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    a_position = glGetAttribLocation(handle_, position.c_str());
    a_normal = glGetAttribLocation(handle_, normal.c_str());
    a_texcoord = glGetAttribLocation(handle_, tex_coord.c_str());
    a_color_multiplier = glGetAttribLocation(handle_, color_multiplier.c_str());
    a_color_offset = glGetAttribLocation(handle_, color_offset.c_str());
}

program_t::~program_t() {
    if (handle_ != 0) {
        glDeleteProgram(handle_);
        gl::check_error();
        handle_ = 0;
    }
}

void program_t::bind_attributes() const {
    int stride = vertex->size;
    int pos_comps = vertex->position_components;

    size_t off = 0;
    GLuint loc = 0;
    if (a_position >= 0) {
        loc = (GLuint) a_position;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, pos_comps, GL_FLOAT, GL_FALSE, stride, (void*) off);
        gl::check_error();
    }
    off += sizeof(float) * pos_comps;

    if (vertex->normals) {
        if (a_normal >= 0) {
            loc = (GLuint) a_normal;
            glEnableVertexAttribArray(loc);
            glVertexAttribPointer(loc, 3, GL_FLOAT, GL_FALSE, stride, (void*) off);
            gl::check_error();
        }
        off += sizeof(float) * 3;
    }

    if (a_texcoord >= 0) {
        loc = (GLuint) a_texcoord;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 2, GL_FLOAT, GL_FALSE, stride, (void*) off);
        gl::check_error();
    }
    off += sizeof(float) * 2;

    if (a_color_multiplier >= 0) {
        loc = (GLuint) a_color_multiplier;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*) off);
        gl::check_error();
    }
    off += sizeof(uint32_t);

    if (a_color_offset >= 0) {
        loc = (GLuint) a_color_offset;
        glEnableVertexAttribArray(loc);
        glVertexAttribPointer(loc, 4, GL_UNSIGNED_BYTE, GL_TRUE, stride, (void*) off);
        gl::check_error();
    }
//    off += sizeof(uint32_t);

}

void program_t::unbind_attributes() const {
    auto loc = (GLuint) a_position;
    glDisableVertexAttribArray(loc);
    gl::check_error();

    if (a_normal >= 0) {
        loc = (GLuint) a_normal;
        glDisableVertexAttribArray(loc);
        gl::check_error();
    }

    if (a_texcoord >= 0) {
        loc = (GLuint) a_texcoord;
        glDisableVertexAttribArray(loc);
        gl::check_error();
    }

    if (a_color_multiplier >= 0) {
        loc = (GLuint) a_color_multiplier;
        glDisableVertexAttribArray(loc);
        gl::check_error();
    }

    if (a_color_offset >= 0) {
        loc = (GLuint) a_color_offset;
        glDisableVertexAttribArray(loc);
        gl::check_error();
    }
}

void program_t::bind_image() const {
    set_uniform(image_0, u_image0_unit);
}

const program_t* current_program = nullptr;

void program_t::use() const {
    if (current_program) {
//        current_program->unbind_attributes();
    }

    glUseProgram(handle_);
    gl::check_error();

    current_program = this;
    if (current_program) {
//        current_program->bind_attributes();
//        current_program->bind_image();
    }
}

GLint program_t::get_uniform(const std::string& name) const {
    GLint uniform;
    auto it = uniforms_cache_.find(name);
    if (it == uniforms_cache_.end()) {
        uniform = glGetUniformLocation(handle_, name.c_str());
        uniforms_cache_[name] = uniform;
    } else {
        uniform = it->second;
    }
    return uniform;
}

void program_t::set_uniform(const std::string& name, const float2& v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniform2fv(uniform, 1, v.data_);
        gl::check_error();
    }
}

void program_t::set_uniform(const std::string& name, const float3& v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniform3fv(uniform, 1, v.data_);
        gl::check_error();
    }
}

void program_t::set_uniform(const std::string& name, const float4& v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniform4fv(uniform, 1, v.data_);
        gl::check_error();
    }
}

void program_t::set_uniform(const std::string& name, const mat4f& v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniformMatrix4fv(uniform, 1, GL_FALSE, v.m);
        gl::check_error();
    }
}

void program_t::set_uniform(const std::string& name, const mat3f& v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniformMatrix3fv(uniform, 1, GL_FALSE, v.m);
        gl::check_error();
    }
}

void program_t::set_uniform(const std::string& name, float v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniform1f(uniform, v);
        gl::check_error();
    }
}

void program_t::set_uniform(const std::string& name, int v) const {
    GLint uniform = get_uniform(name);
    if (uniform >= 0) {
        glUniform1i(uniform, v);
        gl::check_error();
    }
}

}
