#include "export_font.hpp"

namespace ek::font_lib {

using spritepack::atlas_t;
using spritepack::sprite_t;

static ft2_context ft2_context_{};

template<typename K, typename V>
inline V* try_get(std::unordered_map<K, V*>& map, const K& key) {
    auto it = map.find(key);
    return it != map.end() ? it->second : nullptr;
}

template<typename K, typename V>
inline V* try_get(std::unordered_map<K, V>& map, const K& key) {
    auto it = map.find(key);
    return it != map.end() ? &it->second : nullptr;
}

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf,
                             uint8_t* dest_argb32_buf,
                             int pixels_count) {
    for (int i = 0; i < pixels_count; ++i) {
        const uint8_t a = *source_a8_buf;
        dest_argb32_buf[0] = a;
        dest_argb32_buf[1] = a;
        dest_argb32_buf[2] = a;
        dest_argb32_buf[3] = a;
        dest_argb32_buf += 4;
        ++source_a8_buf;
    }
}

BitmapFontGlyph build_glyph_data(const ft2_face& face_managed, uint32_t glyph_index, const std::string& name) {
    BitmapFontGlyph data{};
    int box[4]{};
    face_managed.get_glyph_metrics(glyph_index, box, &data.advance_width);
    int2 lt{box[0], -box[3]};
    int2 rb{box[2], -box[1]};
    data.box = {lt, rb - lt};
    data.sprite = name + std::to_string(glyph_index);
    return data;
}

image_t* render_glyph_image(const ft2_face& face_managed, uint32_t glyph_index, rect_i& out_rect) {
    auto face = face_managed.data();
    auto err = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
    if (err != 0) {
        return nullptr;
    }

    err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);
    if (err != 0) {
        return nullptr;
    }

    auto const& bitmap = face->glyph->bitmap;

    if (bitmap.buffer == nullptr) {
        return nullptr;
    }

    assert(bitmap.width != 0 && bitmap.rows != 0);
    out_rect = face_managed.get_glyph_bounds(face);

    auto pixels_count = bitmap.width * bitmap.rows;
    auto* img = new image_t(bitmap.width, bitmap.rows);
    convert_a8_to_argb32pma(bitmap.buffer,
                            img->data(),
                            pixels_count);

    return img;
}

void glyph_build_sprites(ft2_face& face,
                         uint32_t glyph_index,
                         const std::string& name,
                         const std::vector<uint16_t>& sizes,
                         const std::vector<filter_data_t>& filters,
                         atlas_t& to_atlas) {

    assert(!sizes.empty());

    float size0 = sizes[0];
    for (auto size: sizes) {
        const std::string ref = name + std::to_string(glyph_index) + '_' + std::to_string(size);
        for (auto& resolution : to_atlas.resolutions) {
            const float scale = resolution.resolution_scale;
            face.set_glyph_size(size, scale);
            const auto filter_scale = scale * (static_cast<float>(size) / size0);
            const auto filters_scaled = apply_scale(filters, filter_scale);

            rect_i rect;
            image_t* image = render_glyph_image(face, glyph_index, rect);
            if (image) {
                sprite_t sprite;
                sprite.name = ref;
                sprite.image = image;
                sprite.rc = rect * (1.0f / scale);

                // TODO: preserve RC / SOURCE
//                sprite.source = {
//                        0, 0,
//                        static_cast<int>(image->width()),
//                        static_cast<int>(image->height())
//                };
                sprite.source = rect;

                if (!filters_scaled.empty()) {
                    flash::apply(filters_scaled, sprite, scale);
                }

                sprite.source.x = 0;
                sprite.source.y = 0;

                resolution.sprites.push_back(sprite);
            }
        }
    }
}

BitmapFontData export_font(const path_t& path,
                           const std::string& name,
                           const font_decl_t& font_opts,
                           const filters_decl_t& filters_opts,
                           atlas_t& to_atlas) {

    ft2_face face_managed{ft2_context_, path.str()};
    auto face = face_managed.data();
//            var simplify = opts.simplify != null ? opts.simplify : 0;
    std::unordered_map<uint32_t, BitmapFontGlyph> mesh_by_glyph_id;
    std::unordered_map<uint32_t, BitmapFontGlyph*> char_to_data;

    auto char_codes = face_managed.get_available_char_codes(font_opts.ranges);

    for (FT_ULong char_code : char_codes) {
        FT_UInt glyph_index = FT_Get_Char_Index(face, char_code);
        auto* tmp = try_get(mesh_by_glyph_id, glyph_index);
        if (tmp == nullptr) {
            auto data = build_glyph_data(face_managed, glyph_index, name);
            mesh_by_glyph_id[glyph_index] = data;
            glyph_build_sprites(face_managed, glyph_index,
                                name, font_opts.sizes,
                                filters_opts.filters,
                                to_atlas);
        }
        auto* gdata = try_get(mesh_by_glyph_id, glyph_index);
        gdata->codes.push_back(static_cast<uint32_t>(char_code));
        char_to_data[char_code] = gdata;
    }

    if (FT_HAS_KERNING(face) != 0 && font_opts.use_kerning) {
        for (auto& id_glyph : mesh_by_glyph_id) {
            auto glyph_left = id_glyph.first;
            for (auto& glyph_p2 : mesh_by_glyph_id) {
                auto glyph_right = glyph_p2.first;
                if (glyph_left != glyph_right) {
                    FT_Vector kern;
                    FT_Get_Kerning(face, glyph_left, glyph_right, FT_KERNING_UNSCALED, &kern);
                }
            }
        }

    }

    if (font_opts.mirror_case) {
        for (auto& id_glyph : mesh_by_glyph_id) {
            for (auto code : id_glyph.second.codes) {
                auto upper = (uint32_t) toupper(code);
                auto lower = (uint32_t) tolower(code);
                if (lower != upper) {
                    if (try_get(char_to_data, lower) == nullptr) {
                        id_glyph.second.codes.push_back(lower);
                    } else if (try_get(char_to_data, upper) == nullptr) {
                        id_glyph.second.codes.push_back(upper);
                    }
                }
            }
        }
    }

    BitmapFontData result{};
    result.units_per_em = face->units_per_EM;
    result.sizes = font_opts.sizes;
    for (auto& pair : mesh_by_glyph_id) {
        result.glyphs.push_back(pair.second);
    }
    return result;
}

}
