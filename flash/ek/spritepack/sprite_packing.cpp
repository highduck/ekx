#include "sprite_packing.hpp"
#include <ek/math/max_rects.hpp>
#include <ek/imaging/drawing.hpp>

namespace ek::spritepack {

rect_i no_pack_padding(binpack::rect_t rect, int pad) {
    return {
            rect.x + pad,
            rect.y + pad,
            rect.width - 2 * pad,
            rect.height - 2 * pad
    };
}

rect_f calc_uv(rect_i source, float2 atlas_size, bool rotated) {
    if (rotated) {
        std::swap(source.width, source.height);
    }
    return source / atlas_size;
}


std::vector<atlas_page_t> pack(std::vector<sprite_t> sprites, const int2 max_size) {
    std::vector<atlas_page_t> pages;

//    timer timer{};
//    EK_DEBUG("Packing %lu sprites...", sprites.size());

    bool need_to_pack = true;
    while (need_to_pack) {
        binpack::packer_state_t packer_state{max_size.x, max_size.y};

        for (auto& sprite: sprites) {
            if (!sprite.is_packed()) {
                packer_state.add(
                        sprite.source.width,
                        sprite.source.height,
                        sprite.padding,
                        &sprite);
            }
        }

        need_to_pack = !packer_state.empty();
        if (need_to_pack) {
            pack_nodes(packer_state);

            atlas_page_t page{};
            page.size = {packer_state.canvas.width, packer_state.canvas.height};
            page.image = new image_t(page.size.x, page.size.y);
            const float2 atlas_page_size_f{
                    static_cast<float>(page.size.x),
                    static_cast<float>(page.size.y)
            };

            {
                //atlas_renderer_cairo renderer{*page.image};
                for (size_t i = 0; i < packer_state.rects.size(); ++i) {
                    if (!packer_state.is_packed(i)) {
                        continue;
                    }
                    auto& sprite = packer_state.get_user_data<sprite_t>(i);
                    sprite.enable(sprite_flags_t::packed);
                    if (packer_state.is_rotated(i)) {
                        sprite.enable(sprite_flags_t::rotated);
                    }

                    const rect_i packed_rect = no_pack_padding(packer_state.get_rect(i), sprite.padding);
                    if (sprite.is_rotated()) {
                        copy_pixels_ccw_90(*page.image, packed_rect.position, *sprite.image, sprite.source);
                    } else {
                        copy_pixels_normal(*page.image, packed_rect.position, *sprite.image, sprite.source);
                    }
                    delete sprite.image;
                    sprite.image = page.image;
                    sprite.source = packed_rect;
                    sprite.uv = calc_uv(packed_rect, atlas_page_size_f, sprite.is_rotated());

                    page.sprites.emplace_back(sprite);
                }
            }
            pages.push_back(std::move(page));
        }
    }

//    LOG_PERF("Packed %lu to %lu pages for %lf ms",
//             sprites.size(),
//             pages.size(),
//             get_elapsed_time(timer));

    return pages;
}

std::vector<sprite_t> unpack(const std::vector<atlas_page_t>& pages) {
    std::vector<sprite_t> sprites;
    for (auto& page : pages) {
        for (auto& sprite : page.sprites) {
            sprite_t spr = sprite;
            spr.disable(sprite_flags_t::packed);
            sprites.push_back(spr);
        }
    }
    return sprites;
}

std::vector<atlas_page_t> repack(const std::vector<atlas_page_t>& pages, const int2 max_size) {
    return pack(unpack(pages), max_size);
}

// TODO: atlas
// ++page_index;
// page.image_path = atlas.name + get_atlas_suffix(atlas.scale, page_index) + ".png";

}