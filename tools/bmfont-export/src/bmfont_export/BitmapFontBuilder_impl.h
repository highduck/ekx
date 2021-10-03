#pragma once

#include <cstring>
#include <cassert>
#include "types.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H

namespace bmfont_export {

struct FreeType2 {
    inline static FreeType2& instance() {
        static FreeType2 lib{};
        return lib;
    }

    FreeType2() noexcept {
        auto err = FT_Init_FreeType(&ft);
        if (err != 0) {
            BMFE_LOG_F("FreeType2 init error: %d", err);
        }
    }

    ~FreeType2() {
        FT_Done_FreeType(ft);
    }

    FT_Library ft{};
};

struct FontFaceInfo {
    uint16_t unitsPerEM;
    int16_t ascender;
    int16_t descender;
    int16_t lineHeight;
    bool hasKerning;
};

bool check(const std::vector<CodepointPair>& ranges, uint32_t codepoint) {
    for (const auto& range: ranges) {
        if (codepoint >= range.a && codepoint <= range.b) {
            return true;
        }
    }
    return false;
}

class FontFace {
public:
    FontFace(const FreeType2& lib, const std::string& path) {
        int err = FT_New_Face(lib.ft, path.c_str(), 0, &ftFace);
        if (err != 0) {
            BMFE_LOG_F("FT2 new face error: %d (%s)", err, FT_Error_String(err));
        }
    }

    ~FontFace() {
        FT_Done_Face(ftFace);
    }

    bool getGlyphMetrics(uint32_t glyphIndex, int* out_bbox, int* out_advance) const {
        FT_Error err = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_NO_SCALE);
        if (err == 0) {
            FT_GlyphSlot slot = ftFace->glyph;
            FT_Glyph glyph;
            FT_Get_Glyph(slot, &glyph);

            FT_BBox bbox;
            FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_UNSCALED, &bbox);

            out_bbox[0] = int(bbox.xMin);
            out_bbox[1] = int(bbox.yMin);
            out_bbox[2] = int(bbox.xMax);
            out_bbox[3] = int(bbox.yMax);
            *out_advance = int(slot->advance.x);

            FT_Done_Glyph(glyph);

            return true;
        }
        return false;
    }

    [[nodiscard]]
    std::vector<uint32_t> getAvailableCodepoints(const std::vector<CodepointPair>& ranges) const {
        std::vector<uint32_t> ret{};

        FT_UInt glyphIndex;
        FT_ULong codepoint = FT_Get_First_Char(ftFace, &glyphIndex);
        while (glyphIndex != 0) {
            if (check(ranges, codepoint)) {
                ret.emplace_back(codepoint);
            }
            codepoint = FT_Get_Next_Char(ftFace, codepoint, &glyphIndex);
        }
        return ret;
    }

    bool setGlyphSize(int fontSize, float scaleFactor) {
        // TODO:
        //  - make scaling to match freetype/stb openFrameworks/graphics/ofTrueTypeFont.cpp
        auto dpi = static_cast<uint32_t>(72.0f * scaleFactor);
        return FT_Set_Char_Size(ftFace, 0, fontSize << 6, dpi, dpi) == 0;
    }

    uint32_t getGlyphIndex(uint32_t codepoint) const {
        return FT_Get_Char_Index(ftFace, codepoint);
    }

    bool loadGlyph(uint32_t glyphIndex) const {
        auto err = FT_Load_Glyph(ftFace, glyphIndex, FT_LOAD_DEFAULT);
        return err == 0;
    }

    bool renderGlyph(uint8_t** buffer, uint32_t* width, uint32_t* height) const {
        FT_Stroker stroker;
        FT_BitmapGlyph ft_bitmap_glyph;

        auto& ft2 = FreeType2::instance();
        FT_Error error = FT_Stroker_New(ft2.ft, &stroker);

        if (error) {
            //freetype_error( error );
            //goto cleanup_stroker;
        }

        FT_Stroker_Set(stroker,
                       (int) (0 * 64),
                       FT_STROKER_LINECAP_ROUND,
                       FT_STROKER_LINEJOIN_ROUND,
                       0);
        FT_Glyph glyph = nullptr;
        FT_Get_Glyph(ftFace->glyph, &glyph);
        //error = FT_Glyph_Stroke(&glyph, stroker, 1);
        error = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

        error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
        //auto err = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

        FT_Stroker_Done(stroker);

        if (error == 0) {
//            auto bitmap = ftFace->glyph->bitmap;
            auto bitmap = ((FT_BitmapGlyph) glyph)->bitmap;
            if (bitmap.buffer) {
                assert(bitmap.width != 0 && bitmap.rows != 0);
                if (buffer) {
                    *buffer = bitmap.buffer;
                }
                if (width) {
                    *width = bitmap.width;
                }
                if (height) {
                    *height = bitmap.rows;
                }
                return true;
            }
        }
        return false;
    }

    RectI getGlyphBounds() const {
        auto const& glyph = ftFace->glyph;
        return RectI{glyph->bitmap_left,
                     -glyph->bitmap_top,
                     (int) glyph->bitmap.width,
                     (int) glyph->bitmap.rows};
    }

    bool getKerning(uint32_t index1, uint32_t index2, int* x, int* y) const {
        FT_Vector kern{};
        auto res = FT_Get_Kerning(ftFace, index1, index2, FT_KERNING_UNSCALED, &kern);
        if (res == 0) {
            if (x) {
                *x = kern.x;
            }
            if (y) {
                *y = kern.y;
            }
            return true;
        }
        return false;
    }

    FontFaceInfo getInfo() const {
        FontFaceInfo info{};
        info.unitsPerEM = ftFace->units_per_EM;
        info.ascender = ftFace->ascender;
        info.descender = ftFace->descender;
        info.lineHeight = ftFace->height;
        info.hasKerning = FT_HAS_KERNING(ftFace) != 0;
        return info;
    }

    FT_Face ftFace{};
};

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

Glyph buildGlyphData(const FontFace& fontFace, uint32_t glyph_index, const char* name) {
    Glyph data{};
    int32_t box[4]{};
    fontFace.getGlyphMetrics(glyph_index, box, &data.advanceWidth);
    int l = box[0];
    int t = -box[3];
    int r = box[2];
    int b = -box[1];
    data.box = {l, t, r - l, b - t};
    data.sprite = std::string{name} + std::to_string(glyph_index);
    return data;
}

Bitmap* render_glyph_image(const FontFace& fontFace, uint32_t glyphIndex, RectI& outRect) {
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
    auto* img = new Bitmap(width, height);
    convert_a8_to_argb32pma(buffer, img->data, pixels_count);

    return img;
}

void glyph_build_sprites(FontFace& fontFace,
                         uint32_t glyph_index,
                         const char* name,
                         uint16_t fontSize,
                         const std::vector<Filter>& filters,
                         ImageCollection& toAtlas) {
    const auto ref = std::string{name} + std::to_string(glyph_index);
    for (auto& resolution: toAtlas.resolutions) {
        const float scale = resolution.scale;
        fontFace.setGlyphSize(fontSize, scale);
        const auto filter_scale = scale;
        const auto filters_scaled = apply_scale(filters, filter_scale);

        RectI rect;
        Bitmap* data = render_glyph_image(fontFace, glyph_index, rect);
        if (data) {
            Image image;
            image.name = ref;
            image.bitmap = data;
            image.rc = {
                    (float) rect.x / scale,
                    (float) rect.y / scale,
                    (float) rect.w / scale,
                    (float) rect.h / scale,
            };

            // TODO: preserve RC / SOURCE
//                sprite.source = {
//                        0, 0,
//                        static_cast<int>(image->width()),
//                        static_cast<int>(image->height())
//                };
            image.source = rect;

            if (!filters_scaled.empty()) {
                apply(filters_scaled, image, scale);
            }

            image.source.x = 0;
            image.source.y = 0;

            resolution.images.push_back(image);
        }
    }
}

Font buildBitmapFont(const BuildBitmapFontSettings& decl,
                     ImageCollection& imageCollection) {

    auto& ft2 = FreeType2::instance();
    FontFace fontFace{ft2, decl.ttfPath};
    const auto info = fontFace.getInfo();

    std::unordered_map<uint32_t, Glyph> dataByGlyphIndex;
    std::unordered_map<uint32_t, Glyph*> ptrByCodepoint;

    auto codepoints = fontFace.getAvailableCodepoints(decl.ranges);

    for (auto codepoint: codepoints) {
        auto glyphIndex = fontFace.getGlyphIndex(codepoint);
        auto it = dataByGlyphIndex.find(glyphIndex);
        if (it == dataByGlyphIndex.end()) {
            auto data = buildGlyphData(fontFace, glyphIndex, decl.name);
            dataByGlyphIndex[glyphIndex] = data;
            glyph_build_sprites(fontFace, glyphIndex,
                                decl.name, decl.fontSize,
                                decl.filters,
                                imageCollection);
        }
        auto& gdata = dataByGlyphIndex[glyphIndex];
        gdata.codepoints.push_back(codepoint);
        ptrByCodepoint[codepoint] = &gdata;
    }

    if (info.hasKerning && decl.useKerning && false) {
        for (auto& id_glyph: dataByGlyphIndex) {
            auto glyph_left = id_glyph.first;
            for (auto& glyph_p2: dataByGlyphIndex) {
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
        for (auto& id_glyph: dataByGlyphIndex) {
            for (auto code: id_glyph.second.codepoints) {
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

    Font result{};
    result.unitsPerEM = info.unitsPerEM;
    result.ascender = info.ascender;
    result.descender = info.descender;
    result.lineHeight = info.lineHeight;
    result.fontSize = decl.fontSize;
    for (auto& pair: dataByGlyphIndex) {
        result.glyphs.push_back(pair.second);
    }
    return result;
}

}
