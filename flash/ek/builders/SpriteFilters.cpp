#include "SpriteFilters.hpp"

#include <ek/imaging/drawing.hpp>
#include <ek/builders/MultiResAtlas.hpp>
#include <pugixml.hpp>
#include <ek/util/strings.hpp>

namespace ek {

inline SpriteFilter apply_scale(const SpriteFilter& filter, float scale) {
    SpriteFilter r = filter;
    r.top *= scale;
    r.bottom *= scale;
    r.blur *= scale;
    r.distance *= scale;
    return r;
}

std::vector<SpriteFilter> apply_scale(const std::vector<SpriteFilter>& filters, float scale) {
    std::vector<SpriteFilter> res;
    res.reserve(filters.size());
    for (const auto& filter : filters) {
        res.push_back(apply_scale(filter, scale));
    }
    return res;
}

inline int2 round_blur_radius(const float2& radius) {
    return int2{
            std::max(0, std::min(256, static_cast<int>(radius.x) - 1)),
            std::max(0, std::min(256, static_cast<int>(radius.y) - 1))
    };
}

inline void extend_blur_rect(rect_f& rect, const float2& blur, int pass) {
    auto radius = round_blur_radius(blur);
    // Distination pixels can "move" more left, as these left pixels can take extra from the right
    int extra_x1 = radius.x / 2;
    int extra_x0 = radius.x - extra_x1;
    int extra_y1 = radius.y / 2;
    int extra_y0 = radius.y - extra_y1;

    if (pass & 1u) {
        std::swap(extra_x0, extra_x1);
        std::swap(extra_y0, extra_y1);
    }

    rect.x -= extra_x0;
    rect.y -= extra_y0;
    rect.width += radius.x;
    rect.height += radius.y;
}

inline rect_f extend_blur_rect(const rect_f& rect, const SpriteFilter& filter) {
    rect_f res{rect};
    for (int q = 0; q < filter.quality; ++q) {
        extend_blur_rect(res, filter.blur, q);
    }
    return res;
}

inline rect_f extend_filter_rect(const rect_f& rect, const SpriteFilter& filter) {
    rect_f res{rect};
    switch (filter.type) {
        case SpriteFilterType::Glow:
        case SpriteFilterType::Shadow: {
            const auto offset = filter.distance * float2{cosf(filter.angle), sinf(filter.angle)};
            res = combine(
                    res,
                    translate(
                            extend_blur_rect(res, filter),
                            offset
                    )
            );
        }
            break;
        default:
            break;
    }
    return res;
}

rect_i get_filtered_rect(const rect_i& rc, const std::vector<SpriteFilter>& filters) {
    rect_f res{
            static_cast<float>(rc.x),
            static_cast<float>(rc.y),
            static_cast<float>(rc.width),
            static_cast<float>(rc.height)
    };
    for (auto& filter : filters) {
        res = extend_filter_rect(res, filter);
    }
    const auto l = static_cast<int32_t>(floor(res.x));
    const auto t = static_cast<int32_t>(floor(res.y));
    const auto r = static_cast<int32_t>(ceil(res.right()));
    const auto b = static_cast<int32_t>(ceil(res.bottom()));
    return {l, t, r - l, b - t};
}

uint32_t convert_strength(float value) {
    const auto strength = static_cast<uint32_t>(std::max(value, 0.0f) * 256.0f);
    return std::min(strength, 0x10000u);
}

void apply_strength(image_t& surf, uint32_t strength) {
    if (strength == 0x100) {
        return;
    }

    uint8_t lut[256];
    for (int a = 0; a < 256; a++) {
        uint32_t v = (a * strength) >> 8u;
        lut[a] = v < 255u ? v : 255u;
    }

    for (uint32_t y = 0u; y < surf.height(); ++y) {
        auto* r = surf.row(y);
        for (uint32_t x = 0u; x < surf.width(); ++x) {
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

void apply_color(image_t& surf, argb32_t color) {
    const auto pma_color = argb32_t{
            static_cast<uint8_t>((color.r * color.a * 258u) >> 16u),
            static_cast<uint8_t>((color.g * color.a * 258u) >> 16u),
            static_cast<uint8_t>((color.b * color.a * 258u) >> 16u),
            color.a
    };

    for (uint32_t y = 0u; y < surf.height(); ++y) {
        auto* r = surf.row(y);
        for (uint32_t x = 0u; x < surf.width(); ++x) {
            auto* p = r + x;
            const auto a = static_cast<uint32_t>(p->a) * 258u;
            if (a != 0u) {
                p->a = static_cast<uint8_t>((pma_color.a * a) >> 16u);
                p->r = static_cast<uint8_t>((pma_color.r * a) >> 16u);
                p->g = static_cast<uint8_t>((pma_color.g * a) >> 16u);
                p->b = static_cast<uint8_t>((pma_color.b * a) >> 16u);
            }
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
void BlurRow(const abgr32_t* inSrc, int inDS, int inSrcW, int inFilterLeft,
             abgr32_t* inDest, int inDD, int inDestW, int inFilterSize, int inPixelsLeft) {

    int sr = 0;
    int sg = 0;
    int sb = 0;
    int sa = 0;

    // loop over destination pixels with kernel    -xxx+
    // At each pixel, we - the trailing pixel and + the leading pixel
    const abgr32_t* prev = inSrc - inFilterLeft * inDS;
    const abgr32_t* first = std::max(prev, inSrc - inPixelsLeft * inDS);
    const abgr32_t* src = prev + inFilterSize * inDS;
    const abgr32_t* src_end = inSrc + inSrcW * inDS;

    abgr32_t* dest = inDest;

    for (const abgr32_t* s = first; s < src; s += inDS) {
        sa += s->a;
        sr += s->r;
        sg += s->g;
        sb += s->b;
    }
    for (int x = 0; x < inDestW; x++) {
        if (prev >= src_end) {
            for (; x < inDestW; x++) {
                dest->abgr = 0;
                dest += inDD;
            }
            return;
        }

        if (sa == 0) {
            dest->abgr = 0;
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

void DoApply(const image_t& inSrc,
             image_t& outDest,
             int2 inSrc0,
             int2 inDiff,
             uint32_t inPass,
             int2 blurRadius) {
    uint32_t w = outDest.width();
    uint32_t h = outDest.height();
    uint32_t sw = inSrc.width();
    uint32_t sh = inSrc.height();

    uint32_t blurred_w = std::min(sw + blurRadius.x, w);
    uint32_t blurred_h = std::min(sh + blurRadius.y, h);
    // TODO: tmp height is potentially less (h+mBlurY) than sh ...
    image_t tmp{blurred_w, sh};

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
        sw = tmp.width();
    }

    int s_stride = tmp.width();
    int d_stride = outDest.width();
    // Blur cols ...
    int sy0 = inSrc0.y + inDiff.y;
    for (uint32_t x = 0; x < blurred_w; x++) {
        auto* dest = outDest.row(0) + x;
        const auto* src = tmp.row(sy0) + x;

        BlurRow(src, s_stride, sh - sy0, oy, dest, d_stride, blurred_h, blurRadius.y + 1, sy0);
    }
}

void scroll(const image_t& src, image_t& dst, int2 offset) {
    for (uint32_t y = 0u; y < src.height(); ++y) {
        auto* r = src.row(y);
        auto dstY = y + offset.y;
        if (dstY < 0 || dstY >= dst.height()) continue;
        auto* dstRow = dst.row(y + offset.y);
        for (uint32_t x = 0u; x < src.width(); ++x) {
            auto dstX = x + offset.x;
            if (dstX < 0 || dstX >= dst.width()) continue;
            dstRow[dstX] = r[x];
        }
    }
}

void blur(image_t& src, const SpriteFilter& filter) {
    const auto radius = round_blur_radius(filter.blur);
    image_t tmp{src.width(), src.height()};
    image_t* chainSrc = &src;
    image_t* chainDst = &tmp;
    for (int q = 0; q < filter.quality; ++q) {
        DoApply(*chainSrc, *chainDst, int2::zero, int2::zero, q, radius);
        std::swap(chainDst, chainSrc);
    }
    if (chainSrc == &tmp) {
        src.assign(tmp);
    }
}

void gradient(image_t& src, const SpriteFilter& filter, const rect_i& bounds) {
    for (uint32_t y = 0; y < src.height(); ++y) {
        const float distance = filter.bottom - filter.top;
        const float coord = static_cast<float>(y) + bounds.y;
        const float t = (coord - filter.top) / distance;
        const abgr32_t m = lerp(filter.color, filter.color_bottom, math::clamp(t)).abgr();
        auto* r = src.row(y);
        for (uint32_t x = 0; x < src.width(); ++x) {
            r[x] *= m;
        }
    }
}

void apply(image_t& image, const SpriteFilter& filter, const rect_i& bounds) {
    switch (filter.type) {
        case SpriteFilterType::ReFill:
            gradient(image, filter, bounds);
            break;
        case SpriteFilterType::Glow: {
            image_t tmp{image.width(), image.height()};
            tmp.assign(image);
            blur(tmp, filter);
            apply_strength(tmp, convert_strength(filter.strength));
            apply_color(tmp, filter.color);
            blit(tmp, image);
            image.assign(tmp);
        }
            break;
        case SpriteFilterType::Shadow: {
            image_t tmp{image.width(), image.height()};
            int2 offset{
                    static_cast<int>(filter.distance * cos(filter.angle)),
                    static_cast<int>(filter.distance * sin(filter.angle))
            };
            scroll(image, tmp, offset);
            blur(tmp, filter);
            apply_strength(tmp, convert_strength(filter.strength));
            apply_color(tmp, filter.color);
            blit(tmp, image);
            image.assign(tmp);
        }
            break;
        default:
            break;
    }
}

void apply(const std::vector<SpriteFilter>& filters, SpriteData& sprite, float scale) {
    const rect_i bounds = get_filtered_rect(sprite.source, filters);
    auto res = sprite;
    uint32_t width = bounds.width;
    uint32_t height = bounds.height;
    auto* dest = new image_t(width, height);
    const int2 dest_pos = sprite.source.position - bounds.position;
    image_t& img = *sprite.image;
    copy_pixels_normal(*dest, dest_pos, img, img.bounds<int>());

    for (auto& filter : filters) {
        apply(*dest, filter, bounds);
    }

    res.rc = bounds * (1.0f / scale);
    res.source = bounds;

    res.image = dest;
    // TODO: preserved pixels?
    delete sprite.image;
    sprite.image = nullptr;
    sprite = res;
}


SpriteFilterType get_filter_type(const std::string& type) {
    if (type == "refill") {
        return SpriteFilterType::ReFill;
    } else if (type == "glow") {
        return SpriteFilterType::Glow;
    } else if (type == "shadow") {
        return SpriteFilterType::Shadow;
    }
    return SpriteFilterType::None;
}

std::string filter_type_to_string(SpriteFilterType type) {
    if (type == SpriteFilterType::ReFill) {
        return "refill";
    } else if (type == SpriteFilterType::Glow) {
        return "glow";
    } else if (type == SpriteFilterType::Shadow) {
        return "shadow";
    }
    return "bypass";
}

argb32_t parse_argb32(const std::string& str) {
    uint32_t c = 0x0;
    sscanf(str.c_str(), "#%08x", &c);
    return argb32_t{c};
}

void filterDataFromXML(const pugi::xml_node& node, SpriteFilter& filter) {
    filter.type = get_filter_type(node.attribute("type").as_string());
    filter.top = node.attribute("top").as_float(0.0f);
    filter.bottom = node.attribute("bottom").as_float(100.0f);
    filter.distance = node.attribute("distance").as_float(0.0f);
    filter.quality = node.attribute("quality").as_int(1);
    filter.strength = node.attribute("strength").as_float(1.0f);
    filter.angle = ek::math::to_radians(node.attribute("angle").as_float(0.0f));
    filter.blur.x = node.attribute("blur_x").as_float(4.0f);
    filter.blur.y = node.attribute("blur_y").as_float(4.0f);
    filter.color = parse_argb32(node.attribute("color").as_string("#FF000000"));
    filter.color_bottom = parse_argb32(node.attribute("color_bottom").as_string("#FF000000"));
}

void filterDataToXML(pugi::xml_node& node, const SpriteFilter& filter) {
    node.append_attribute("type").set_value(filter_type_to_string(filter.type).c_str());
    node.append_attribute("top").set_value(filter.top);
    node.append_attribute("bottom").set_value(filter.bottom);
    node.append_attribute("distance").set_value(filter.distance);
    node.append_attribute("quality").set_value(filter.quality);
    node.append_attribute("strength").set_value(filter.strength);
    node.append_attribute("angle").set_value(ek::math::to_degrees(filter.angle));
    node.append_attribute("blur_x").set_value(filter.blur.x);
    node.append_attribute("blur_y").set_value(filter.blur.y);
    node.append_attribute("color").set_value(("#" + to_hex(filter.color.argb)).c_str());
    node.append_attribute("color_bottom").set_value(("#" + to_hex(filter.color_bottom.argb)).c_str());
}

void SpriteFilterList::readFromXML(const pugi::xml_node& node) {
    for (auto& filter_node: node.children("filter")) {
        SpriteFilter f{};
        filterDataFromXML(filter_node, f);
        list.push_back(f);
    }
}

void SpriteFilterList::writeToXML(pugi::xml_node& node) const {
    for (auto& filter: list) {
        auto filter_node = node.append_child("filter");
        filterDataToXML(filter_node, filter);
    }
}

}