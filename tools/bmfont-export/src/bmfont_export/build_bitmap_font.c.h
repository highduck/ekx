#pragma once

#include <ctype.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_STROKER_H

#include <ek/assert.h>
#include <ek/buf.h>
#include "bmfont_export.h"

FT_Library get_freetype(void) {
    static FT_Library lib = {0};
    if (!lib) {
        FT_Error err = FT_Init_FreeType(&lib);
        if (err) {
            log_error("FreeType2 init error: %d", err);
        }
    }

    // TODO:
    // FT_Done_FreeType(ft);
    return lib;
}

typedef struct {
    uint16_t unitsPerEM;
    int16_t ascender;
    int16_t descender;
    int16_t lineHeight;
    bool hasKerning;
} font_face_info_t;

bool check(const codepoint_pair_t* ranges, uint32_t ranges_num, uint32_t codepoint) {
    for (uint32_t i = 0; i < ranges_num; ++i) {
        const codepoint_pair_t range = ranges[i];
        if (codepoint >= range.a && codepoint <= range.b) {
            return true;
        }
    }
    return false;
}

FT_Face font_face_open(const char* path) {
    FT_Face face;
    FT_Library ft = get_freetype();
    FT_Error err = FT_New_Face(ft, path, 0, &face);
    if (err) {
        log_error("FT2 new face error: %d (%s)", err, FT_Error_String(err));
    }
    return face;
}

void font_face_close(FT_Face face) {
    FT_Done_Face(face);
}

bool font_face_glyph_metrics(const FT_Face face, uint32_t glyph_index, int* out_bbox, int* out_advance) {
    FT_Error err = FT_Load_Glyph(face, glyph_index, FT_LOAD_NO_SCALE);
    if (!err) {
        FT_GlyphSlot slot = face->glyph;
        FT_Glyph glyph;
        FT_Get_Glyph(slot, &glyph);

        FT_BBox bbox;
        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_UNSCALED, &bbox);

        out_bbox[0] = (int) bbox.xMin;
        out_bbox[1] = (int) bbox.yMin;
        out_bbox[2] = (int) bbox.xMax;
        out_bbox[3] = (int) bbox.yMax;
        *out_advance = (int) slot->advance.x;

        FT_Done_Glyph(glyph);

        return true;
    }
    return false;
}

uint32_t font_face_available_codepoints(
        FT_Face face, const codepoint_pair_t* ranges, uint32_t ranges_num, uint32_t* out_buf) {
    FT_UInt glyphIndex;
    FT_ULong codepoint = FT_Get_First_Char(face, &glyphIndex);
    uint32_t num = 0;
    while (glyphIndex != 0) {
        if (check(ranges, ranges_num, codepoint)) {
            out_buf[num++] = codepoint;
        }
        codepoint = FT_Get_Next_Char(face, codepoint, &glyphIndex);
    }
    return num;
}

bool font_face_set_glyph_size(FT_Face face, int fontSize, float scaleFactor) {
    // TODO:
    //  - make scaling to match freetype/stb openFrameworks/graphics/ofTrueTypeFont.cpp
    uint32_t dpi = (uint32_t) (72.0f * scaleFactor);
    return FT_Set_Char_Size(face, 0, fontSize << 6, dpi, dpi) == 0;
}

uint32_t font_face_glyph_index(const FT_Face face, uint32_t codepoint) {
    return FT_Get_Char_Index(face, codepoint);
}

bool font_face_load_glyph(const FT_Face face, uint32_t glyphIndex) {
    FT_Error err = FT_Load_Glyph(face, glyphIndex, FT_LOAD_DEFAULT);
    return !err;
}

bool font_face_render_glyph(const FT_Face face, uint8_t** buffer, uint32_t* width, uint32_t* height) {
    FT_Stroker stroker;
    //FT_BitmapGlyph ft_bitmap_glyph;

    FT_Library ft = get_freetype();
    FT_Error error = FT_Stroker_New(ft, &stroker);

    if (error) {
        //freetype_error( error );
        //goto cleanup_stroker;
    }

    FT_Stroker_Set(stroker,
                   (int) (0 * 64),
                   FT_STROKER_LINECAP_ROUND,
                   FT_STROKER_LINEJOIN_ROUND,
                   0);
    FT_Glyph glyph = NULL;
    FT_Get_Glyph(face->glyph, &glyph);
    //error = FT_Glyph_Stroke(&glyph, stroker, 1);
    error = FT_Glyph_StrokeBorder(&glyph, stroker, 0, 1);

    error = FT_Glyph_To_Bitmap(&glyph, FT_RENDER_MODE_NORMAL, 0, 1);
    //auto err = FT_Render_Glyph(ftFace->glyph, FT_RENDER_MODE_NORMAL);

    FT_Stroker_Done(stroker);

    if (error == 0) {
        FT_Bitmap bitmap = ((FT_BitmapGlyph) glyph)->bitmap;
        if (bitmap.buffer) {
            EK_ASSERT(bitmap.width != 0);
            EK_ASSERT(bitmap.rows != 0);
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

irect_t font_face_glyph_bounds(const FT_Face face) {
    FT_GlyphSlot glyph = face->glyph;
    return (irect_t) {.x = glyph->bitmap_left,
            .y = -glyph->bitmap_top,
            .w = (int) glyph->bitmap.width,
            .h = (int) glyph->bitmap.rows};
}

bool font_face_read_kerning(const FT_Face face, uint32_t index1, uint32_t index2, int* x, int* y) {
    FT_Vector kern = {0};
    FT_Error res = FT_Get_Kerning(face, index1, index2, FT_KERNING_UNSCALED, &kern);
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

font_face_info_t font_face_info(FT_Face face) {
    font_face_info_t info = {0};
    info.unitsPerEM = face->units_per_EM;
    info.ascender = face->ascender;
    info.descender = face->descender;
    info.lineHeight = face->height;
    info.hasKerning = FT_HAS_KERNING(face) != 0;
    return info;
}

glyph_t build_glyph_data(FT_Face face, uint32_t glyph_index) {
    int32_t box[4] = {0};
    int advance_x = 0;
    font_face_glyph_metrics(face, glyph_index, box, &advance_x);
    const float units_per_em = (float) face->units_per_EM;
    const float inv_units_per_em = 1.0f / units_per_em;

    glyph_t data = {0};
    data.advance_x = inv_units_per_em * advance_x;
    const float l = (float) box[0];
    const float t = (float) -box[3];
    const float r = (float) box[2];
    const float b = (float) -box[1];
    data.box = rect_scale_f(rect(l, t, r - l, b - t), inv_units_per_em);
    data.glyph_index = glyph_index;
    return data;
}

bitmap_t render_glyph_image(FT_Face face, uint32_t glyphIndex, irect_t* outRect) {
    bitmap_t img = {0};
    if (!font_face_load_glyph(face, glyphIndex)) {
        return img;
    }

    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t* buffer = NULL;
    if (!font_face_render_glyph(face, &buffer, &width, &height)) {
        return img;
    }

    *outRect = font_face_glyph_bounds(face);

    const uint32_t pixels_count = width * height;
    bitmap_alloc(&img, width, height);
    convert_a8_to_argb32pma(buffer, (uint32_t*) img.pixels, pixels_count);

    return img;
}

void glyph_build_sprites(FT_Face face,
                         uint32_t glyph_index,
                         uint16_t fontSize,
                         const bitmap_filter_t* filters,
                         uint32_t filters_num,
                         image_set_t* dest_atlas) {
    for (uint32_t i = 0; i < dest_atlas->resolutions_num; ++i) {
        resolution_t* resolution = dest_atlas->resolutions + i;
        const float scale = resolution->scale;
        font_face_set_glyph_size(face, fontSize, scale);
        bitmap_filter_t filters_scaled[32];
        if (filters_num) {
            const float filter_scale = scale;
            for (uint32_t fi = 0; fi < filters_num; ++fi) {
                filters_scaled[fi] = filters[fi];
            }
            bitmap_filters_copy_scale(filters, filters_scaled, filters_num, filter_scale);
        }

        irect_t rect;
        bitmap_t data = render_glyph_image(face, glyph_index, &rect);
        if (data.pixels) {
            image_t image;
            image.glyph_index = glyph_index;
            image.bitmap = data;
            image.padding = 1;
            image.rc = rect_scale_f(irect_to_rect(rect), 1.0f / scale);

            // TODO: preserve RC / SOURCE
//                sprite.source = {
//                        0, 0,
//                        static_cast<int>(image->width()),
//                        static_cast<int>(image->height())
//                };
            image.source = rect;

            if (filters_num) {
                apply_bitmap_filters(filters_scaled, filters_num, scale, &image.rc, &image.source, &image.bitmap);
            }

            image.source.x = 0;
            image.source.y = 0;

            arr_push(&resolution->images, image_t, image);
        }
    }
}

glyph_t* find_data_by_glyph_index(glyph_t* glyphs, uint32_t glyph_index) {
    const uint32_t len = ek_buf_length(glyphs);
    for (uint32_t i = 0; i < len; ++i) {
        if (glyphs[i].glyph_index == glyph_index) {
            return glyphs + i;
        }
    }
    return NULL;
}

glyph_t* find_data_by_codepoint(glyph_t* glyphs, uint32_t codepoint) {
    const uint32_t len = ek_buf_length(glyphs);
    for (uint32_t i = 0; i < len; ++i) {
        glyph_t* glyph = glyphs + i;
        const uint32_t* codepoints = glyph->codepoints;
        const uint32_t codepoints_num = ek_buf_length(codepoints);
        for (uint32_t j = 0; j < codepoints_num; ++j) {
            if (codepoints[j] == codepoint) {
                return glyph;
            }
        }
    }
    return NULL;
}

font_t build_bitmap_font(const bitmap_font_build_options_t* decl, image_set_t* dest_image_set) {
    FT_Face face = font_face_open(decl->ttf_path);
    const font_face_info_t info = font_face_info(face);

    font_t result = {0};
    result.ascender = (float) info.ascender / (float) info.unitsPerEM;
    result.descender = (float) info.descender / (float) info.unitsPerEM;
    result.lineHeight = (float) info.lineHeight / (float) info.unitsPerEM;
    result.fontSize = (float) decl->font_size;

    uint32_t codepoints[2048];
    uint32_t codepoints_num = font_face_available_codepoints(face, decl->ranges, decl->ranges_num, codepoints);

    for (uint32_t i = 0; i < codepoints_num; ++i) {
        uint32_t codepoint = codepoints[i];
        uint32_t glyph_index = font_face_glyph_index(face, codepoint);
        glyph_t* glyph = find_data_by_glyph_index(result.glyphs, glyph_index);
        if (!glyph) {
            glyph_t data = build_glyph_data(face, glyph_index);
            glyph_build_sprites(face, glyph_index, decl->font_size, decl->filters, decl->filters_num, dest_image_set);
            arr_push(&result.glyphs, glyph_t, data);
            glyph = result.glyphs + ek_buf_length(result.glyphs) - 1;
        }
        arr_push(&glyph->codepoints, uint32_t, codepoint);
    }

    if (info.hasKerning && decl->use_kerning && false) {
        const uint32_t glyphs_len = ek_buf_length(result.glyphs);
        for (uint32_t i = 0; i < glyphs_len; ++i) {
            const uint32_t glyph_left = result.glyphs[i].glyph_index;
            for (uint32_t j = 0; j < glyphs_len; ++j) {
                const uint32_t glyph_right = result.glyphs[j].glyph_index;
                int x = 0;
                if (font_face_read_kerning(face, glyph_left, glyph_right, &x, NULL)) {
                    // TODO: add Bitmap Font kerning table
                }
            }
        }
    }

    // use mirrored letter case for missing glyphs
    if (decl->mirror_case) {
        glyph_t* glyphs = result.glyphs;
        uint32_t glyphs_len = ek_buf_length(glyphs);
        for (uint32_t gi = 0; gi < glyphs_len; ++gi) {
            uint32_t** arr_codepoints = &glyphs[gi].codepoints;
            for (uint32_t i = 0, len = ek_buf_length(*arr_codepoints); i < len; ++i) {
                const uint32_t code = (*arr_codepoints)[i];
                const uint32_t upper = (uint32_t) toupper((int) code);
                const uint32_t lower = (uint32_t) tolower((int) code);
                if (lower != upper) {
                    glyph_t* glyph_lower = find_data_by_codepoint(glyphs, lower);
                    glyph_t* glyph_upper = find_data_by_codepoint(glyphs, upper);
                    if (!glyph_lower) {
                        arr_push(arr_codepoints, uint32_t, lower);
                    } else if (!glyph_upper) {
                        arr_push(arr_codepoints, uint32_t, upper);
                    }
                }
            }
        }
    }
    return result;
}
