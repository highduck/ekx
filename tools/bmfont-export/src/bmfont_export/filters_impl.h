#pragma once

#include "types.h"
#include <cmath>

namespace bmfont_export {

Rect combineBounds(const Rect& a, const Rect& b) {
    const float l = a.x < b.x ? a.x : b.x;
    const float t = a.y < b.y ? a.y : b.y;
    const float r = (a.x + a.w) > (b.x + b.w) ? (a.x + a.w) : (b.x + b.w);
    const float bo = (a.y + a.h) > (b.y + b.h) ? (a.y + a.h) : (b.y + b.h);
    return {l, t, r - l, bo - t};
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

inline Vec2I round_blur_radius(float rx, float ry) {
    return Vec2I{
            std::max(0, std::min(256, static_cast<int>(rx) - 1)),
            std::max(0, std::min(256, static_cast<int>(ry) - 1))
    };
}

inline void extend_blur_rect(Rect& rect, const float blurX, const float blurY, int pass) {
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

inline Rect extend_blur_rect(const Rect& rect, const Filter& filter) {
    Rect res{rect};
    for (int q = 0; q < filter.quality; ++q) {
        extend_blur_rect(res, filter.blurX, filter.blurY, q);
    }
    return res;
}

inline Rect extend_filter_rect(const Rect& rect, const Filter& filter) {
    Rect res{rect};
    switch (filter.type) {
        case FilterType::Glow:
        case FilterType::Shadow: {
            const auto rads = (float)(filter.angle * M_PI / 180.0);
            const auto offsetX = filter.distance * cosf(rads);
            const auto offsetY = filter.distance * sinf(rads);
            auto r2 = extend_blur_rect(res, filter);
            r2.x += offsetX;
            r2.y += offsetY;
            res = combineBounds(res, r2);
        }
            break;
        default:
            break;
    }
    return res;
}

RectI get_filtered_rect(const RectI& rc, const std::vector<Filter>& filters) {
    Rect res{
            static_cast<float>(rc.x),
            static_cast<float>(rc.y),
            static_cast<float>(rc.w),
            static_cast<float>(rc.h)
    };
    for (auto& filter: filters) {
        res = extend_filter_rect(res, filter);
    }
    const auto l = static_cast<int32_t>(floor(res.x));
    const auto t = static_cast<int32_t>(floor(res.y));
    const auto r = static_cast<int32_t>(ceil(res.x + res.w));
    const auto b = static_cast<int32_t>(ceil(res.y + res.h));
    return {l, t, r - l, b - t};
}

uint32_t convert_strength(float value) {
    const auto strength = static_cast<uint32_t>(std::max(value, 0.0f) * 256.0f);
    return std::min(strength, 0x10000u);
}

void apply_strength(Bitmap& surf, uint32_t strength) {
    if (strength == 0x100) {
        return;
    }

    uint8_t lut[256];
    for (int a = 0; a < 256; a++) {
        uint32_t v = (a * strength) >> 8u;
        lut[a] = v < 255u ? v : 255u;
    }

    for (uint32_t y = 0u; y < surf.h; ++y) {
        auto* r = surf.row(y);
        for (uint32_t x = 0u; x < surf.w; ++x) {
            const auto a0 = r[x].a;
            if (a0 != 0) {
                const auto a1 = lut[a0];
                const float k = static_cast<float>(a1) / static_cast<float>(a0);
                r[x].a = a1;
                r[x].r = static_cast<uint8_t>(r[x].r * k);
                r[x].g = static_cast<uint8_t>(r[x].g * k);
                r[x].b = static_cast<uint8_t>(r[x].b * k);
            }
        }
    }
}

void apply_color(Bitmap& surf, Color color) {
    color.r *= color.a;
    color.g *= color.a;
    color.b *= color.a;
    const Rgba pmaRgba = toRGBA(color);

    for (int y = 0u; y < surf.h; ++y) {
        auto* r = surf.row(y);
        for (int x = 0u; x < surf.w; ++x) {
            const auto a = static_cast<uint32_t>(r->a) * 258u;
            if (a != 0u) {
                r->r = static_cast<uint8_t>((a * pmaRgba.r) >> 16u);
                r->g = static_cast<uint8_t>((a * pmaRgba.g) >> 16u);
                r->b = static_cast<uint8_t>((a * pmaRgba.b) >> 16u);
                r->a = static_cast<uint8_t>((a * pmaRgba.a) >> 16u);
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
void BlurRow(const Rgba* inSrc, int inDS, int inSrcW, int inFilterLeft,
             Rgba* inDest, int inDD, int inDestW, int inFilterSize, int inPixelsLeft) {

    int sr = 0;
    int sg = 0;
    int sb = 0;
    int sa = 0;

    // loop over destination pixels with kernel    -xxx+
    // At each pixel, we - the trailing pixel and + the leading pixel
    const Rgba* prev = inSrc - inFilterLeft * inDS;
    const Rgba* first = std::max(prev, inSrc - inPixelsLeft * inDS);
    const Rgba* src = prev + inFilterSize * inDS;
    const Rgba* src_end = inSrc + inSrcW * inDS;

    Rgba* dest = inDest;

    for (const Rgba* s = first; s < src; s += inDS) {
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

void DoApply(const Bitmap& inSrc,
             Bitmap& outDest,
             Vec2I inSrc0,
             Vec2I inDiff,
             uint32_t inPass,
             Vec2I blurRadius) {
    int w = outDest.w;
    int h = outDest.h;
    int sw = inSrc.w;
    int sh = inSrc.h;

    int blurred_w = std::min(sw + blurRadius.x, w);
    int blurred_h = std::min(sh + blurRadius.y, h);
    // TODO: tmp height is potentially less (h+mBlurY) than sh ...
    Bitmap tmp{blurred_w, sh};

    int ox = blurRadius.x / 2;
    int oy = blurRadius.y / 2;
    if ((inPass & 1u) == 0u) {
        ox = blurRadius.x - ox;
        oy = blurRadius.y - oy;
    }

    {
        // Blur rows ...
        int sx0 = inSrc0.x + inDiff.x;
        for (uint32_t y = 0; y < sh; y++) {
            auto* dest = tmp.row(y);
            const auto* src = inSrc.row(y) + sx0;

            BlurRow(src, 1, sw - sx0, ox, dest, 1, blurred_w, blurRadius.x + 1, sx0);
        }
        sw = tmp.w;
    }

    int s_stride = tmp.w;
    int d_stride = outDest.w;
    // Blur cols ...
    int sy0 = inSrc0.y + inDiff.y;
    for (uint32_t x = 0; x < blurred_w; x++) {
        auto* dest = outDest.row(0) + x;
        const auto* src = tmp.row(sy0) + x;

        BlurRow(src, s_stride, sh - sy0, oy, dest, d_stride, blurred_h, blurRadius.y + 1, sy0);
    }
}

void scroll(const Bitmap& src, Bitmap& dst, Vec2I offset) {
    for (uint32_t y = 0u; y < src.h; ++y) {
        auto* r = src.row(y);
        auto dstY = y + offset.y;
        if (dstY < 0 || dstY >= dst.h) continue;
        auto* dstRow = dst.row(y + offset.y);
        for (uint32_t x = 0u; x < src.w; ++x) {
            auto dstX = x + offset.x;
            if (dstX < 0 || dstX >= dst.w) continue;
            dstRow[dstX] = r[x];
        }
    }
}

void blur(Bitmap& src, const Filter& filter) {
    const auto radius = round_blur_radius(filter.blurX, filter.blurY);
    Bitmap tmp{src.w, src.h};
    Bitmap* chainSrc = &src;
    Bitmap* chainDst = &tmp;
    for (int q = 0; q < filter.quality; ++q) {
        DoApply(*chainSrc, *chainDst, {0, 0}, {0, 0}, q, radius);
        std::swap(chainDst, chainSrc);
    }
    if (chainSrc == &tmp) {
        src.assign(tmp);
    }
}

void gradient(Bitmap& src, const Filter& filter, const RectI& bounds) {
    for (int y = 0; y < src.h; ++y) {
        const float distance = filter.bottom - filter.top;
        const float coord = static_cast<float>(y) + bounds.y;
        const float t = (coord - filter.top) / distance;
        const Rgba m = toRGBA(lerp(filter.color0, filter.color1, saturate(t)));
        auto* r = src.row(y);
        for (int x = 0; x < src.w; ++x) {
            r[x] *= m;
        }
    }
}

void apply(Bitmap& bitmap, const Filter& filter, const RectI& bounds) {
    switch (filter.type) {
        case FilterType::ReFill:
            gradient(bitmap, filter, bounds);
            break;
        case FilterType::Glow: {
            Bitmap tmp{bitmap.w, bitmap.h};
            tmp.assign(bitmap);
            blur(tmp, filter);
            apply_strength(tmp, convert_strength(filter.strength));
            apply_color(tmp, filter.color0);
            blit(tmp, bitmap);
            bitmap.assign(tmp);
        }
            break;
        case FilterType::Shadow: {
            Bitmap tmp{bitmap.w, bitmap.h};
            Vec2I offset{
                    static_cast<int>(filter.distance * cos(filter.angle)),
                    static_cast<int>(filter.distance * sin(filter.angle))
            };
            scroll(bitmap, tmp, offset);
            blur(tmp, filter);
            apply_strength(tmp, convert_strength(filter.strength));
            apply_color(tmp, filter.color0);
            blit(tmp, bitmap);
            bitmap.assign(tmp);
        }
            break;
        default:
            break;
    }
}

void apply(const std::vector<Filter>& filters, Image& image, float scale) {
    const RectI bounds = get_filtered_rect(image.source, filters);
    auto res = image;
    auto* dest = new Bitmap(bounds.w, bounds.h);
    const Vec2I dest_pos{
            image.source.x - bounds.x,
            image.source.y - bounds.y
    };
    Bitmap* idata = image.bitmap;
    copyPixels(*dest, dest_pos.x, dest_pos.y, *idata, 0, 0, idata->w, idata->h);

    for (auto& filter: filters) {
        apply(*dest, filter, bounds);
    }

    res.rc = {
            (float) bounds.x / scale,
            (float) bounds.y / scale,
            (float) bounds.w / scale,
            (float) bounds.h / scale,
    };
    res.source = bounds;

    res.bitmap = dest;
    // TODO: preserved pixels?
    delete image.bitmap;
    image.bitmap = nullptr;
    image = res;
}

}