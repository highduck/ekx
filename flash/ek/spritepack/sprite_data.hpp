#pragma once

#include "atlas_declaration.hpp"

#include <ek/math/box.hpp>
#include <string>
#include <utility>
#include <vector>
#include <ek/imaging/image.hpp>
#include <ek/math/serialize_math.hpp>
#include <ek/graphics/texture.hpp>

namespace ek::spritepack {

enum class sprite_flags_t : uint8_t {
    none = 0u,
    rotated = 1u,
    packed = 2u
};

struct sprite_t {

    std::string name;

    // physical rect
    rect_f rc;

    // coords in atlas image
    rect_f uv;

    // flags in atlas image
    uint8_t flags = 0u;

    // rect in source image
    rect_i source;

    uint8_t padding = 1;

    // reference image;
    image_t* image = nullptr;

    [[nodiscard]]
    bool is_packed() const {
        return (flags & static_cast<uint8_t>(sprite_flags_t::packed)) != 0;
    }

    [[nodiscard]]
    bool is_rotated() const {
        return (flags & static_cast<uint8_t>(sprite_flags_t::rotated)) != 0;
    }

    void enable(sprite_flags_t flag) {
        flags |= static_cast<uint8_t>(flag);
    }

    void disable(sprite_flags_t flag) {
        flags &= ~static_cast<uint8_t>(flag);
    }

    template<typename S>
    void serialize(IO<S>& io) {
        io(name, rc, uv, flags);
    }
};

struct atlas_page_t {
    vec2_t<uint16_t> size;
    std::vector<sprite_t> sprites;
    std::string image_path;
    image_t* image = nullptr;
    texture_t* texture = nullptr;

    template<typename S>
    void serialize(IO<S>& io) {
        io(size, image_path, sprites);
    }
};

struct atlas_resolution_t {
    float resolution_scale = 1.0f;
    int resolution_index = 0;
    int2 max_size{2048, 2048};
    std::vector<sprite_t> sprites;
    std::vector<atlas_page_t> pages;

    atlas_resolution_t() = default;

    explicit atlas_resolution_t(const atlas_resolution_decl_t& decl, int index = 0)
            : resolution_scale{decl.scale},
              resolution_index{index},
              max_size{decl.max_size} {

    }

    template<typename S>
    void serialize(IO<S>& io) {
        io(pages);
    }
};

struct atlas_t {
    std::string name;
    std::vector<atlas_resolution_t> resolutions;

    atlas_t() {
        resolutions.resize(4);
        for (size_t i = 0; i < resolutions.size(); ++i) {
            resolutions[i].resolution_index = i;
            resolutions[i].resolution_scale = float(i);
        }
    }

    explicit atlas_t(const atlas_decl_t& decl) {
        int i = 0;
        resolutions.reserve(decl.resolutions.size());
        for (const auto& resolution : decl.resolutions) {
            resolutions.emplace_back(resolution, i);
            ++i;
        }
        name = decl.name;
    }

    ~atlas_t() {
        for (auto& res : resolutions) {
            for (auto& page : res.pages) {
                if (page.image) {
                    delete page.image;
                    page.image = nullptr;
                }
            }
        }
    }
};

}