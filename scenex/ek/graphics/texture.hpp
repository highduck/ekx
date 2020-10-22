#pragma once

#include <ek/util/common_macro.hpp>
#include <ek/imaging/image.hpp>
#include "gl_def.hpp"

#include <array>

namespace ek::graphics {

enum class texture_type {
    color32,
    depth16,
    depth24
};

class texture_t : private disable_copy_assign_t {
    friend class render_target_t;

public:

    texture_t();

    explicit texture_t(bool is_cube_map);

    ~texture_t();

    void reset(uint32_t width, uint32_t height, texture_type type = texture_type::color32);

    void upload(const image_t& image);

    void upload_cubemap(const std::array<image_t*, 6>& image);

    void upload_pixels(uint32_t width, uint32_t height, const uint32_t* pixels_rgba32);

    void bind(int unit) const;

    [[nodiscard]] inline uint32_t handle() const {
        return handle_;
    }

    [[nodiscard]] inline texture_type type() const {
        return type_;
    }

private:
    GLuint handle_ = 0u;
    texture_type type_;
    GLenum gl_texture_target_;
    bool is_cube_map_;
};

void load_texture_lazy(const char* path, texture_t* to_texture);
void load_texture_cube_lazy(const std::vector<std::string>& path_list, texture_t* to_texture);

}