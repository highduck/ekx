#pragma once

#include "types.h"

namespace bmfont_export {

rect_t combineBounds(const rect_t a, const rect_t b) {
    const float l = a.x < b.x ? a.x : b.x;
    const float t = a.y < b.y ? a.y : b.y;
    const float r = (a.x + a.w) > (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const float bo = (a.y + a.h) > (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return rect(l, t, r - l, bo - t);
}

inline Filter apply_scale(const Filter& filter, float scale) {
    Filter r = filter;
    r.top *= scale;
    r.bottom *= scale;
    r.blurX *= scale;
    r.blurY *= scale;
    r.distance *= scale;
    return r;
}

std::vector<Filter> apply_scale(const std::vector<Filter>& filters, float scale) {
    std::vector<Filter> res(filters.size());
    for (const auto& filter: filters) {
        res.push_back(apply_scale(filter, scale));
    }
    return res;
}

inline ivec2_t round_blur_radius(float rx, float ry) {
    return ivec2(
            MAX(0, MIN(256, (int) rx - 1)),
            MAX(0, MIN(256, (int) ry - 1))
    );
}

inline void extend_blur_rect(rect_t& rect, const float blurX, const float blurY, int pass) {
    auto radius = round_blur_radius(blurX, blurY);
    // Distination pixels can "move" more left, as these left pixels can take extra from the right
    int extra_x1 = radius.x / 2;
    int extra_x0 = radius.x - extra_x1;
    int extra_y1 = radius.y / 2;
    int extra_y0 = radius.y - extra_y1;

    if (pass & 1u) {
        std::swap(extra_x0, extra_x1);
        std::swap(extra_y0, extra_y1);
    }

    rect.x -= (float) extra_x0;
    rect.y -= (float) extra_y0;
    rect.w += (float) radius.x;
    rect.h += (float) radius.y;
}

inline rect_t extend_blur_rect(const rect_t rect, const Filter& filter) {
    rect_t res = rect;
    for (int q = 0; q < filter.quality; ++q) {
        extend_blur_rect(res, filter.blurX, filter.blurY, q);
    }
    return res;
}

inline rect_t extend_filter_rect(const rect_t rect, const Filter& filter) {
    rect_t res{rect};
    switch (filter.type) {
        case FilterType::Glow:
        case FilterType::Shadow: {
            const vec2_t offset = vec2_cs(to_radians(filter.angle));
            auto r2 = extend_blur_rect(res, filter);
            r2.x += offset.x;
            r2.y += offset.y;
            res = combineBounds(res, r2);
        }
            break;
        default:
            break;
    }
    return res;
}

irect_t get_filtered_rect(const irect_t rc, const std::vector<Filter>& filters) {
    rect_t res = irect_to_rect(rc);
    for (auto& filter: filters) {
        res = extend_filter_rect(res, filter);
    }
    const auto l = (int32_t) (floor(res.x));
    const auto t = (int32_t) (floor(res.y));
    const auto r = (int32_t) (ceil(res.x + res.w));
    const auto b = (int32_t) (ceil(res.y + res.h));
    return (irect_t) {{l, t, r - l, b - t}};
}

uint32_t convert_strength(float value) {
    const auto strength = static_cast<uint32_t>(std::max(value, 0.0f) * 256.0f);
    return std::min(strength, 0x10000u);
}

void apply_strength(ek_bitmap surf, uint32_t strength) {
    if (strength == 0x100) {
        return;
    }

    uint8_t lut[256];
    for (int a = 0; a < 256; a++) {
        uint32_t v = (a * strength) >> 8u;
        lut[a] = v < 255u ? v : 255u;
    }

    for (int y = 0; y < surf.h; ++y) {
        color_t* r = ek_bitmap_row(surf, y);
        for (int x = 0; x < surf.w; ++x) {
            const auto a0 = r[x].a;
            if (a0 != 0) {
                const auto a1 = lut[a0];
                const float k = static_cast<float>(a1) / static_cast<float>(a0);
                r[x].a = a1;
                r[x].r = (uint8_t) ((float) r[x].r * k);
                r[x].g = (uint8_t) ((float) r[x].g * k);
                r[x].b = (uint8_t) ((float) r[x].b * k);
            }
        }
    }
}

void apply_color(ek_bitmap surf, vec4_t color) {
    color.r *= color.a;
    color.g *= color.a;
    color.b *= color.a;
    const color_t pmaRgba = color_vec4(color);

    for (int y = 0u; y < surf.h; ++y) {
        color_t* r = ek_bitmap_row(surf, y);
        for (int x = 0u; x < surf.w; ++x) {
            const auto a = ((uint32_t) r->a) * 258u;
            if (a != 0u) {
                r->r = (uint8_t) ((a * pmaRgba.r) >> 16u);
                r->g = (uint8_t) ((a * pmaRgba.g) >> 16u);
                r->b = (uint8_t) ((a * pmaRgba.b) >> 16u);
                r->a = (uint8_t) ((a * pmaRgba.a) >> 16u);
            }
            ++r;
        }
    }
}


/*
  inSrc        - src pixel corresponding to first output pixel
  inDS         - pixel stride
  inSrcW       - number of valid source pixels after inSrc
  inFilterLeft - filter size on left
  inDest       - first output pixel
  inDD         - output pixel stride
  inDest       - number of pixels to render

  inFilterSize - total filter size
  inPixelsLeft - number of valid pixels on left
*/
void BlurRow(const color_t* inSrc, int inDS, int inSrcW, int inFilterLeft,
             color_t* inDest, int inDD, int inDestW, int inFilterSize, int inPixelsLeft) {

    int sr = 0;
    int sg = 0;
    int sb = 0;
    int sa = 0;

    // loop over destination pixels with kernel    -xxx+
    // At each pixel, we - the trailing pixel and + the leading pixel
    const color_t* prev = inSrc - inFilterLeft * inDS;
    const color_t* first = std::max(prev, inSrc - inPixelsLeft * inDS);
    const color_t* src = prev + inFilterSize * inDS;
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
            dest->r = sr / inFilterSize;
            dest->g = sg / inFilterSize;
            dest->b = sb / inFilterSize;
            dest->a = sa / inFilterSize;
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

void DoApply(const ek_bitmap inSrc,
             ek_bitmap outDest,
             ivec2_t inSrc0,
             ivec2_t inDiff,
             uint32_t inPass,
             ivec2_t blurRadius) {
    int w = outDest.w;
    int h = outDest.h;
    int sw = inSrc.w;
    int sh = inSrc.h;

    int blurred_w = std::min(sw + blurRadius.x, w);
    int blurred_h = std::min(sh + blurRadius.y, h);
    // TODO: tmp height is potentially less (h+mBlurY) than sh ...
    ek_bitmap tmp = {};
    ek_bitmap_alloc(&tmp, blurred_w, sh);

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
            auto* dest = ek_bitmap_row(tmp, y);
            const auto* src = ek_bitmap_row(inSrc, y) + sx0;

            BlurRow(src, 1, sw - sx0, ox, dest, 1, blurred_w, blurRadius.x + 1, sx0);
        }
        sw = tmp.w;
    }

    int s_stride = tmp.w;
    int d_stride = outDest.w;
    // Blur cols ...
    int sy0 = inSrc0.y + inDiff.y;
    for (int x = 0; x < blurred_w; x++) {
        auto* dest = ek_bitmap_row(outDest, 0) + x;
        const auto* src = ek_bitmap_row(tmp, sy0) + x;

        BlurRow(src, s_stride, sh - sy0, oy, dest, d_stride, blurred_h, blurRadius.y + 1, sy0);
    }
}

void scroll(const ek_bitmap src, ek_bitmap dst, ivec2_t offset) {
    for (int y = 0u; y < src.h; ++y) {
        auto* r = ek_bitmap_row(src, y);
        auto dstY = y + offset.y;
        if (dstY < 0 || dstY >= dst.h) continue;
        auto* dstRow = ek_bitmap_row(dst, y + offset.y);
        for (int x = 0u; x < src.w; ++x) {
            auto dstX = x + offset.x;
            if (dstX < 0 || dstX >= dst.w) continue;
            dstRow[dstX] = r[x];
        }
    }
}

void blur(ek_bitmap src, const Filter& filter) {
    const auto radius = round_blur_radius(filter.blurX, filter.blurY);
    ek_bitmap tmp = {};
    ek_bitmap_alloc(&tmp, src.w, src.h);
    ek_bitmap chainSrc = src;
    ek_bitmap chainDst = tmp;
    for (int q = 0; q < filter.quality; ++q) {
        DoApply(chainSrc, chainDst, {}, {}, q, radius);
        std::swap(chainDst, chainSrc);
    }
    if (chainSrc.pixels == tmp.pixels) {
        ek_bitmap_copy(src, tmp);
    }
    ek_bitmap_free(&tmp);
}

void gradient(ek_bitmap src, const Filter& filter, const irect_t& bounds) {
    for (int y = 0; y < src.h; ++y) {
        const float distance = filter.bottom - filter.top;
        const float coord = (float) y + (float) bounds.y;
        const float t = (coord - filter.top) / distance;
        const color_t m = color_vec4(lerp_vec4(filter.color0, filter.color1, saturate(t)));
        color_t* r = ek_bitmap_row(src, y);
        for (int x = 0; x < src.w; ++x) {
            r[x] = r[x] * m;
        }
    }
}

void apply(ek_bitmap bitmap, const Filter& filter, const irect_t bounds) {
    switch (filter.type) {
        case FilterType::ReFill:
            gradient(bitmap, filter, bounds);
            break;
        case FilterType::Glow: {
            ek_bitmap tmp = {};
            ek_bitmap_clone(&tmp, bitmap);
            blur(bitmap, filter);
            apply_strength(bitmap, convert_strength(filter.strength));
            apply_color(bitmap, filter.color0);
            blit(bitmap, tmp);
            ek_bitmap_free(&tmp);
        }
            break;
        case FilterType::Shadow: {
            ek_bitmap tmp = {};
            ek_bitmap_alloc(&tmp, bitmap.w, bitmap.h);

            vec2_t offset = filter.distance * vec2_cs(to_radians(filter.angle));
            ivec2_t ioffset = {{(int) offset.x, (int) offset.y}};
            scroll(bitmap, tmp, ioffset);
            blur(tmp, filter);
            apply_strength(tmp, convert_strength(filter.strength));
            apply_color(tmp, filter.color0);
            blit(tmp, bitmap);
            ek_bitmap_copy(bitmap, tmp);
            ek_bitmap_free(&tmp);
        }
            break;
        default:
            break;
    }
}

void apply(const std::vector<Filter>& filters, Image& image, float scale) {
    const irect_t bounds = get_filtered_rect(image.source, filters);
    auto res = image;
    ek_bitmap dest = {};
    ek_bitmap_alloc(&dest, bounds.w, bounds.h);
    const ivec2_t dest_pos = ivec2(
            image.source.x - bounds.x,
            image.source.y - bounds.y
    );
    ek_bitmap idata = image.bitmap;
    bitmap_copy(&dest, dest_pos.x, dest_pos.y, &idata, 0, 0, idata.w, idata.h);

    for (auto& filter: filters) {
        apply(dest, filter, bounds);
    }

    res.rc = irect_to_rect(bounds) / scale;
    res.source = bounds;
    res.bitmap = dest;
    // TODO: preserved pixels?
    ek_bitmap_free(&image.bitmap);
    image = res;
}

}