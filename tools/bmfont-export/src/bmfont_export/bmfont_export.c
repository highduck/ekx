#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_ONLY_BMP
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif

#include <stb/stb_image_write.h>
#include "bmfont_export.h"
#include "ek/buf.h"

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf,
                             uint32_t* dest_argb32_buf,
                             int pixels_count) {
    for (int i = 0; i < pixels_count; ++i) {
        const uint32_t a = *source_a8_buf;
        *dest_argb32_buf = (a << 24) | (a << 16) | (a << 8) | a;
        ++dest_argb32_buf;
        ++source_a8_buf;
    }
}

void convert_a8_to_argb32(uint8_t const* source_a8_buf,
                          uint32_t* dest_argb32_buf,
                          int pixels_count) {
    for (int i = 0; i < pixels_count; ++i) {
        const uint32_t a = *source_a8_buf;
        *dest_argb32_buf = (a << 24) | 0xFFFFFF;
        ++dest_argb32_buf;
        ++source_a8_buf;
    }
}

//irect_t clampBounds(const irect_t& a, const irect_t& b) {
//    const int l = a.x > b.x ? a.x : b.x;
//    const int t = a.y > b.y ? a.y : b.y;
//    const int r = (a.x + a.w) < (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
//    const int bo = (a.y + a.h) < (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
//    return {{l, t, r - l, bo - t}};
//}
//
//void clipRects(const irect_t& src_bounds, const irect_t& dest_bounds,
//               irect_t& src_rect, irect_t& dest_rect) {
//    const irect_t src_rc = clampBounds(src_bounds, src_rect);
//    const irect_t dest_rc = clampBounds(dest_bounds, dest_rect);
//    src_rect = dest_rc;
//    src_rect.x += src_rc.x - dest_rect.x;
//    src_rect.y += src_rc.y - dest_rect.y;
//    dest_rect = dest_rc;
//}


// bitmap filter to / from string

bitmap_filter_type bitmap_filter_type_from_string(const char* name) {
    if (!strcmp(name, "refill")) {
        return BITMAP_FILTER_REFILL;
    } else if (!strcmp(name, "glow")) {
        return BITMAP_FILTER_GLOW;
    } else if (!strcmp(name, "shadow")) {
        return BITMAP_FILTER_SHADOW;
    }
    return BITMAP_FILTER_NONE;
}

const char* bitmap_filter_type_to_string(bitmap_filter_type type) {
    if (type == BITMAP_FILTER_REFILL) {
        return "refill";
    } else if (type == BITMAP_FILTER_GLOW) {
        return "glow";
    } else if (type == BITMAP_FILTER_SHADOW) {
        return "shadow";
    }
    return "bypass";
}

rect_t combine_bounds(const rect_t a, const rect_t b) {
    const float l = a.x < b.x ? a.x : b.x;
    const float t = a.y < b.y ? a.y : b.y;
    const float r = (a.x + a.w) > (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const float bo = (a.y + a.h) > (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return rect(l, t, r - l, bo - t);
}

bitmap_filter_t bitmap_filter(void) {
    bitmap_filter_t filter = (bitmap_filter_t) {
            .type = BITMAP_FILTER_NONE,
            .blurX = 0.0f,
            .blurY = 0.0f,
            .strength = 1.0f,
            .quality = 1,
            .color0 = vec4(0, 0, 0, 1),
            .color1 = vec4(1, 1, 1, 1),

            // TODO: check we change it from radian to degree in serialization, we need change calculations for usages
            .angle = 0.0f,
            .distance = 0.0f,

            // refill options
            .top = 0.0f,
            .bottom = 100.0f,

            // flags
            .knockout = false,
            .innerGlow = false,
            .hideObject = false,
    };
    return filter;
}

void parse_bitmap_filter(FILE* f, bitmap_filter_t* filter) {
    fscanf(f, "%u\n", &filter->type);
    fscanf(f, "%f\n", &filter->top); // 0
    fscanf(f, "%f\n", &filter->bottom); // 100
    fscanf(f, "%f\n", &filter->distance);
    fscanf(f, "%u\n", &filter->quality); // 1
    fscanf(f, "%f\n", &filter->strength); // 1
    fscanf(f, "%f\n", &filter->angle);
    fscanf(f, "%f\n", &filter->blurX);
    fscanf(f, "%f\n", &filter->blurY);
    uint32_t color0;
    uint32_t color1;
    fscanf(f, "%u\n", &color0);
    fscanf(f, "%u\n", &color1);
    filter->color0 = vec4_color(color_u32(color0));
    filter->color1 = vec4_color(color_u32(color1));
}

void parse_bitmap_font_build_options(FILE* f, bitmap_font_build_options_t* options) {
    uint32_t font_size = 16;
    uint32_t codepoint_ranges = 0;
    uint32_t resolutions_num = 0;
    uint32_t filters_num = 0;
    uint32_t mirror_case = 0;
    uint32_t use_kerning = 0;
    fscanf(f, "%u\n", &font_size);
    fscanf(f, "%u\n", &codepoint_ranges);
    fscanf(f, "%u\n", &resolutions_num);
    fscanf(f, "%u\n", &filters_num);
    fscanf(f, "%u\n", &mirror_case);
    fscanf(f, "%u\n", &use_kerning);
    char atlas_name[128];
    fscanf(f, "%s\n", atlas_name);

    options->resolutions_num = 0;
    options->mirror_case = mirror_case != 0;
    options->use_kerning = use_kerning != 0;
    options->font_size = font_size;

    for (uint32_t i = 0; i < codepoint_ranges; ++i) {
        uint32_t from;
        uint32_t to;
        fscanf(f, "%u %u\n", &from, &to);
        options->ranges[options->ranges_num++] = (codepoint_pair_t) {from, to};
    }

    for (uint32_t i = 0; i < filters_num; ++i) {
        parse_bitmap_filter(f, options->filters + (options->filters_num++));
    }

    for (uint32_t i = 0; i < resolutions_num; ++i) {
        float scale;
        fscanf(f, "%f\n", &scale);
        options->resolutions[options->resolutions_num++] = scale;
    }
}

bitmap_filter_t bitmap_filter_scale(const bitmap_filter_t filter, float scale) {
    bitmap_filter_t r = filter;
    r.top *= scale;
    r.bottom *= scale;
    r.blurX *= scale;
    r.blurY *= scale;
    r.distance *= scale;
    return r;
}

ivec2_t round_blur_radius(float rx, float ry) {
    return ivec2(
            MAX(0, MIN(256, (int) rx - 1)),
            MAX(0, MIN(256, (int) ry - 1))
    );
}

void extend_blur_rect(rect_t* rect, const float blurX, const float blurY, int pass) {
    ivec2_t radius = round_blur_radius(blurX, blurY);
// Distination pixels can "move" more left, as these left pixels can take extra from the right
    int extra_x1 = radius.x / 2;
    int extra_x0 = radius.x - extra_x1;
    int extra_y1 = radius.y / 2;
    int extra_y0 = radius.y - extra_y1;

    if (pass & 1u) {
        int tmp = extra_x0;
        extra_x0 = extra_x1;
        extra_x1 = tmp;

        tmp = extra_y0;
        extra_y0 = extra_y1;
        extra_y1 = tmp;
    }

    rect->x -= (float) extra_x0;
    rect->y -= (float) extra_y0;
    rect->w += (float) radius.x;
    rect->h += (float) radius.y;
}

rect_t extend_blur_rect_filter(const rect_t rect, const bitmap_filter_t* filter) {
    rect_t res = rect;
    for (int q = 0; q < filter->quality; ++q) {
        extend_blur_rect(&res, filter->blurX, filter->blurY, q);
    }
    return res;
}

rect_t extend_filter_rect(const rect_t rect, const bitmap_filter_t* filter) {
    rect_t res = rect;
    switch (filter->type) {
        case BITMAP_FILTER_GLOW:
        case BITMAP_FILTER_SHADOW: {
            const vec2_t offset = vec2_cs(to_radians(filter->angle));
            rect_t r2 = extend_blur_rect_filter(res, filter);
            r2.x += offset.x;
            r2.y += offset.y;
            res = combine_bounds(res, r2);
        }
            break;
        default:
            break;
    }
    return res;
}

/** PRIVATE FILTERS **/


uint32_t convert_strength(float value) {
    const float m = value * 256.0f;
    const uint32_t strength = (uint32_t) MAX(m, 0);
    return MIN(strength, 0x10000u);
}

void apply_color(bitmap_t surf, vec4_t color) {
    color.r *= color.a;
    color.g *= color.a;
    color.b *= color.a;
    const color_t pma_rgba = color_vec4(color);
    for (int y = 0u; y < surf.h; ++y) {
        color_t* r = bitmap_row(surf, y);
        for (int x = 0u; x < surf.w; ++x) {
            const uint32_t a = ((uint32_t) r->a) * 258u;
            if (a != 0u) {
                r->r = (uint8_t) ((a * pma_rgba.r) >> 16u);
                r->g = (uint8_t) ((a * pma_rgba.g) >> 16u);
                r->b = (uint8_t) ((a * pma_rgba.b) >> 16u);
                r->a = (uint8_t) ((a * pma_rgba.a) >> 16u);
            }
            ++r;
        }
    }
}

void apply_strength(bitmap_t surf, uint32_t strength) {
    if (strength == 0x100) {
        return;
    }

    uint8_t lut[256];
    for (int a = 0; a < 256; a++) {
        uint32_t v = (a * strength) >> 8u;
        lut[a] = v < 255u ? v : 255u;
    }

    for (int y = 0; y < surf.h; ++y) {
        color_t* r = bitmap_row(surf, y);
        for (int x = 0; x < surf.w; ++x) {
            const uint8_t a0 = r[x].a;
            if (a0 != 0) {
                const uint8_t a1 = lut[a0];
                const float k = (float) (a1) / (float) (a0);
                r[x].a = a1;
                r[x].r = (uint8_t) ((float) r[x].r * k);
                r[x].g = (uint8_t) ((float) r[x].g * k);
                r[x].b = (uint8_t) ((float) r[x].b * k);
            }
        }
    }
}


/*
  inSrc        - src pixel corresponding to first output pixel
  inDS         - pixel stride
  inSrcW       - number of valid source pixels after inSrc
  inbitmap_filter_tLeft - filter size on left
  inDest       - first output pixel
  inDD         - output pixel stride
  inDest       - number of pixels to render

  inbitmap_filter_tSize - total filter size
  inPixelsLeft - number of valid pixels on left
*/
void BlurRow(const color_t* inSrc, int inDS, int inSrcW, int inbitmap_filter_tLeft,
             color_t* inDest, int inDD, int inDestW, int inbitmap_filter_tSize, int inPixelsLeft) {

    int sr = 0;
    int sg = 0;
    int sb = 0;
    int sa = 0;

    // loop over destination pixels with kernel    -xxx+
    // At each pixel, we - the trailing pixel and + the leading pixel
    const color_t* prev = inSrc - inbitmap_filter_tLeft * inDS;
    const color_t* first = MAX(prev, inSrc - inPixelsLeft * inDS);
    const color_t* src = prev + inbitmap_filter_tSize * inDS;
    const color_t* src_end = inSrc + inSrcW * inDS;

    color_t* dest = inDest;

    for (const color_t* s = first; s < src; s += inDS) {
        sa += s->a;
        sr += s->r;
        sg += s->g;
        sb += s->b;
    }
    for (int x = 0; x < inDestW; x++) {
        if (prev >= src_end) {
            for (; x < inDestW; x++) {
                *(uint32_t*) dest = 0;
                dest += inDD;
            }
            return;
        }

        if (sa == 0) {
            *(uint32_t*) dest = 0;
        } else {
            dest->r = sr / inbitmap_filter_tSize;
            dest->g = sg / inbitmap_filter_tSize;
            dest->b = sb / inbitmap_filter_tSize;
            dest->a = sa / inbitmap_filter_tSize;
        }

        if (src >= inSrc && src < src_end) {
            sa += src->a;
            sr += src->r;
            sg += src->g;
            sb += src->b;
        }

        if (prev >= first) {
            sa -= prev->a;
            sr -= prev->r;
            sg -= prev->g;
            sb -= prev->b;
        }

        src += inDS;
        prev += inDS;
        dest += inDD;
    }
}

void DoApply(const bitmap_t inSrc,
             bitmap_t outDest,
             ivec2_t inSrc0,
             ivec2_t inDiff,
             uint32_t inPass,
             ivec2_t blurRadius) {
    int w = outDest.w;
    int h = outDest.h;
    int sw = inSrc.w;
    int sh = inSrc.h;

    int blurred_w = sw + blurRadius.x;
    blurred_w = MIN(blurred_w, w);
    int blurred_h = sh + blurRadius.y;
    blurred_h = MIN(blurred_h, h);

    // TODO: tmp height is potentially less (h+mBlurY) than sh ...
    bitmap_t tmp = {0};
    bitmap_alloc(&tmp, blurred_w, sh);

    int ox = blurRadius.x / 2;
    int oy = blurRadius.y / 2;
    if ((inPass & 1u) == 0u) {
        ox = blurRadius.x - ox;
        oy = blurRadius.y - oy;
    }

    {
        // Blur rows ...
        int sx0 = inSrc0.x + inDiff.x;
        for (int y = 0; y < sh; y++) {
            color_t* dest = bitmap_row(tmp, y);
            const color_t* src = bitmap_row(inSrc, y) + sx0;

            BlurRow(src, 1, sw - sx0, ox, dest, 1, blurred_w, blurRadius.x + 1, sx0);
        }
        sw = tmp.w;
    }

    int s_stride = tmp.w;
    int d_stride = outDest.w;
    // Blur cols ...
    int sy0 = inSrc0.y + inDiff.y;
    for (int x = 0; x < blurred_w; x++) {
        color_t* dest = bitmap_row(outDest, 0) + x;
        const color_t* src = bitmap_row(tmp, sy0) + x;

        BlurRow(src, s_stride, sh - sy0, oy, dest, d_stride, blurred_h, blurRadius.y + 1, sy0);
    }
}

void scroll(const bitmap_t src, bitmap_t dst, ivec2_t offset) {
    for (int y = 0u; y < src.h; ++y) {
        color_t* r = bitmap_row(src, y);
        int dstY = y + offset.y;
        if (dstY < 0 || dstY >= dst.h) continue;
        color_t* dstRow = bitmap_row(dst, y + offset.y);
        for (int x = 0u; x < src.w; ++x) {
            int dstX = x + offset.x;
            if (dstX < 0 || dstX >= dst.w) continue;
            dstRow[dstX] = r[x];
        }
    }
}

void blur(bitmap_t src, const bitmap_filter_t* filter) {
    const ivec2_t radius = round_blur_radius(filter->blurX, filter->blurY);
    bitmap_t tmp = {0};
    bitmap_alloc(&tmp, src.w, src.h);
    bitmap_t chainSrc = src;
    bitmap_t chainDst = tmp;
    for (int q = 0; q < filter->quality; ++q) {
        DoApply(chainSrc, chainDst, (ivec2_t) {0}, (ivec2_t) {0}, q, radius);
        // std::swap(chainDst, chainSrc);
        bitmap_t tmp2 = chainDst;
        chainDst = chainSrc;
        chainSrc = tmp2;
    }
    if (chainSrc.pixels == tmp.pixels) {
        bitmap_copy(src, tmp);
    }
    bitmap_free(&tmp);
}

void gradient(bitmap_t src, const bitmap_filter_t* filter, const irect_t bounds) {
    for (int y = 0; y < src.h; ++y) {
        const float distance = filter->bottom - filter->top;
        const float coord = (float) y + (float) bounds.y;
        const float t = (coord - filter->top) / distance;
        const color_t m = color_vec4(lerp_vec4(filter->color0, filter->color1, saturate(t)));
        color_t* r = bitmap_row(src, y);
        for (int x = 0; x < src.w; ++x) {
            r[x] = mul_color(r[x], m);
        }
    }
}

void apply_bitmap_filter(bitmap_t bitmap, const bitmap_filter_t* filter, const irect_t bounds) {
    switch (filter->type) {
        case BITMAP_FILTER_REFILL:
            gradient(bitmap, filter, bounds);
            break;
        case BITMAP_FILTER_GLOW: {
            bitmap_t tmp = {0};
            bitmap_clone(&tmp, bitmap);
            blur(bitmap, filter);
            apply_strength(bitmap, convert_strength(filter->strength));
            apply_color(bitmap, filter->color0);
            bitmap_blit(bitmap, tmp);
            bitmap_free(&tmp);
        }
            break;
        case BITMAP_FILTER_SHADOW: {
            bitmap_t tmp = {0};
            bitmap_alloc(&tmp, bitmap.w, bitmap.h);

            vec2_t offset = vec2_cs(to_radians(filter->angle));
            offset = scale_vec2(offset, filter->distance);

            ivec2_t ioffset = (ivec2_t) {.x =(int) offset.x, .y= (int) offset.y};
            scroll(bitmap, tmp, ioffset);
            blur(tmp, filter);
            apply_strength(tmp, convert_strength(filter->strength));
            apply_color(tmp, filter->color0);
            bitmap_blit(tmp, bitmap);
            bitmap_copy(bitmap, tmp);
            bitmap_free(&tmp);
        }
            break;
        default:
            break;
    }
}


void bitmap_filters_copy_scale(const bitmap_filter_t* src, bitmap_filter_t* dest, uint32_t n, float scale) {
    for (uint32_t i = 0; i < n; ++i) {
        dest[i] = bitmap_filter_scale(src[i], scale);
    }
}

irect_t get_filtered_rect(const irect_t rc, const bitmap_filter_t* filters, uint32_t n) {
    rect_t res = irect_to_rect(rc);
    for (uint32_t i = 0; i < n; ++i) {
        res = extend_filter_rect(res, filters + i);
    }
    const int l = (int) (floorf(res.x));
    const int t = (int) (floorf(res.y));
    const int r = (int) (ceilf(res.x + res.w));
    const int b = (int) (ceilf(res.y + res.h));
    return (irect_t) {.x = l, .y = t, .w = r - l, .h = b - t};
}


void apply_bitmap_filters(const bitmap_filter_t* filters, uint32_t n, float scale, rect_t* rc, irect_t* source, bitmap_t* bitmap) {
    const irect_t bounds = get_filtered_rect(*source, filters, n);
    //auto res = image;
    bitmap_t dest = {0};
    bitmap_alloc(&dest, bounds.w, bounds.h);
    const ivec2_t dest_pos = ivec2(
            source->x - bounds.x,
            source->y - bounds.y
    );
    bitmap_t idata = *bitmap;
    bitmap_blit_copy(&dest, dest_pos.x, dest_pos.y, &idata, 0, 0, idata.w, idata.h);

    for (uint32_t i = 0; i < n; ++i) {
        apply_bitmap_filter(dest, filters + i, bounds);
    }

    // TODO: preserved pixels?
    bitmap_free(bitmap);

    *rc = rect_scale_f(irect_to_rect(bounds), scale);
    *source = bounds;
    *bitmap = dest;
}

////// FILTERS END

void save_font_bitmaps(const char* font_name, const image_set_t* images, const char* output) {
    char path[1024];
    snprintf(path, sizeof path, "%s/images.txt", output);
    FILE* f = fopen(path, "wb");
    int idx = 0;

    for (uint32_t i = 0; i < images->resolutions_num; ++i) {
        const resolution_t* resolution = images->resolutions + i;
        uint32_t bitmaps_num = 0;
        uint32_t images_len = ek_buf_length(resolution->images);
        for (uint32_t ii = 0; ii < images_len; ++ii) {
            if (resolution->images[ii].bitmap.pixels) {
                ++bitmaps_num;
            }
        }
        fprintf(f, "%u\n", bitmaps_num);
        for (uint32_t ii = 0; ii < images_len; ++ii) {
            image_t* image = resolution->images + ii;
            if (image->bitmap.pixels) {
                snprintf(path, sizeof path, "%s/%d.bmp", output, idx++);
                const char* imagePath = path;
                fprintf(f, "%s%u\n%s\n%f %f %f %f %u %u\n",
                        font_name, image->glyph_index,
                        imagePath,
                        image->rc.x, image->rc.y, image->rc.w, image->rc.h,
                        image->padding, 0);
                {
                    bitmap_t bitmap = image->bitmap;
                    // require RGBA non-premultiplied alpha
                    bitmap_unpremultiply(&bitmap);
                    stbi_write_bmp(imagePath, bitmap.w, bitmap.h, 4, bitmap.pixels);
                }
            }
        }
    }

    fclose(f);
}
