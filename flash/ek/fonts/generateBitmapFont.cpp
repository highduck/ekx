#include <ek/util/strings.hpp>
#include "generateBitmapFont.hpp"

#include "ft_library.hpp"

namespace ek::font_lib {

void GenBitmapFontDecl::readFromXML(const pugi::xml_node& node) {
    fontSize = node.attribute("fontSize").as_int(16);

    const std::pair<uint32_t, uint32_t> defaultCodepointRange{0x0020u, 0x007Fu};
    for (auto& rangeNode: node.children("code_range")) {
        auto cr = defaultCodepointRange;
        cr.first = std::strtoul(rangeNode.attribute("from").as_string("0x0020"), nullptr, 16);
        cr.second = std::strtoul(rangeNode.attribute("to").as_string("0x007F"), nullptr, 16);
        ranges.push_back(cr);
    }

    if (ranges.empty()) {
        ranges.push_back(defaultCodepointRange);
    }

    mirrorCase = node.attribute("mirror_case").as_bool(false);
    useKerning = node.attribute("use_kerning").as_bool(true);
}

void GenBitmapFontDecl::writeToXML(pugi::xml_node& node) const {
    node.append_attribute("fontSize").set_value(fontSize);

    for (const auto& range: ranges) {
        auto range_node = node.append_child("code_range");
        range_node.append_attribute("from").set_value(("0x" + to_hex(range.first)).c_str());
        range_node.append_attribute("to").set_value(("0x" + to_hex(range.second)).c_str());
    }

    node.append_attribute("mirror_case").set_value(mirrorCase);
    node.append_attribute("use_kerning").set_value(useKerning);
}

using spritepack::atlas_t;
using spritepack::sprite_t;

static FreeType2 ft2{};

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

BitmapFontGlyph build_glyph_data(const FontFace& fontFace, uint32_t glyph_index, const std::string& name) {
    BitmapFontGlyph data{};
    int box[4]{};
    fontFace.getGlyphMetrics(glyph_index, box, &data.advanceWidth);
    int2 lt{box[0], -box[3]};
    int2 rb{box[2], -box[1]};
    data.box = {lt, rb - lt};
    data.sprite = name + std::to_string(glyph_index);
    return data;
}

image_t* render_glyph_image(const FontFace& fontFace, uint32_t glyphIndex, rect_i& outRect) {
    if (!fontFace.loadGlyph(glyphIndex)) {
        return nullptr;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t* buffer = nullptr;
    if (!fontFace.renderGlyph(&buffer, &width, &height)) {
        return nullptr;
    }

    outRect = fontFace.getGlyphBounds();

    auto pixels_count = width * height;
    auto* img = new image_t(width, height);
    convert_a8_to_argb32pma(buffer, img->data(), pixels_count);

    return img;
}

void glyph_build_sprites(FontFace& fontFace,
                         uint32_t glyph_index,
                         const std::string& name,
                         uint16_t fontSize,
                         const std::vector<filter_data_t>& filters,
                         atlas_t& to_atlas) {
    const std::string ref = name + std::to_string(glyph_index);
    for (auto& resolution : to_atlas.resolutions) {
        const float scale = resolution.resolution_scale;
        fontFace.setGlyphSize(fontSize, scale);
        const auto filter_scale = scale;
        const auto filters_scaled = apply_scale(filters, filter_scale);

        rect_i rect;
        image_t* image = render_glyph_image(fontFace, glyph_index, rect);
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
                apply(filters_scaled, sprite, scale);
            }

            sprite.source.x = 0;
            sprite.source.y = 0;

            resolution.sprites.push_back(sprite);
        }
    }
}

BitmapFontData generateBitmapFont(const path_t& path,
                                  const std::string& name,
                                  const GenBitmapFontDecl& decl,
                                  const FiltersDecl& filters,
                                  atlas_t& to_atlas) {

    FontFace fontFace{ft2, path.str()};
    const auto info = fontFace.getInfo();

    std::unordered_map<uint32_t, BitmapFontGlyph> dataByGlyphIndex;
    std::unordered_map<uint32_t, BitmapFontGlyph*> ptrByCodepoint;

    auto codepoints = fontFace.getAvailableCodepoints(decl.ranges);

    for (auto codepoint : codepoints) {
        auto glyphIndex = fontFace.getGlyphIndex(codepoint);
        auto it = dataByGlyphIndex.find(glyphIndex);
        if (it == dataByGlyphIndex.end()) {
            auto data = build_glyph_data(fontFace, glyphIndex, name);
            dataByGlyphIndex[glyphIndex] = data;
            glyph_build_sprites(fontFace, glyphIndex,
                                name, decl.fontSize,
                                filters.filters,
                                to_atlas);
        }
        auto& gdata = dataByGlyphIndex[glyphIndex];
        gdata.codepoints.push_back(codepoint);
        ptrByCodepoint[codepoint] = &gdata;
    }

    if (info.hasKerning && decl.useKerning && false) {
        for (auto& id_glyph : dataByGlyphIndex) {
            auto glyph_left = id_glyph.first;
            for (auto& glyph_p2 : dataByGlyphIndex) {
                auto glyph_right = glyph_p2.first;
                if (glyph_left != glyph_right) {
                    int x = 0;
                    if (fontFace.getKerning(glyph_left, glyph_right, &x, nullptr)) {
                        // TODO: add Bitmap Font kerning table
                    }
                }
            }
        }
    }

    if (decl.mirrorCase) {
        for (auto& id_glyph : dataByGlyphIndex) {
            for (auto code : id_glyph.second.codepoints) {
                auto upper = (uint32_t) toupper(code);
                auto lower = (uint32_t) tolower(code);
                if (lower != upper) {
                    if (try_get(ptrByCodepoint, lower) == nullptr) {
                        id_glyph.second.codepoints.push_back(lower);
                    } else if (try_get(ptrByCodepoint, upper) == nullptr) {
                        id_glyph.second.codepoints.push_back(upper);
                    }
                }
            }
        }
    }

    BitmapFontData result{};
    result.unitsPerEM = info.unitsPerEM;
    result.ascender = info.ascender;
    result.descender = info.descender;
    result.lineHeight = info.lineHeight;
    result.fontSize = decl.fontSize;
    for (auto& pair : dataByGlyphIndex) {
        result.glyphs.push_back(pair.second);
    }
    return result;
}

}
