#pragma once

#include <cstdint>

namespace bmfont_export {

struct Color {
    float r;
    float g;
    float b;
    float a;
};

Color lerp(Color a, Color b, float t) {
    const float i = 1.0f - t;
    return {
            a.r * i + b.r * t,
            a.g * i + b.g * t,
            a.b * i + b.b * t,
            a.a * i + b.a * t,
    };
}

struct alignas(4) Rgba {
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
};

inline Rgba operator*(Rgba x, Rgba multiplier) {
    return Rgba{
            static_cast<uint8_t>((x.r * multiplier.r * 258u) >> 16u),
            static_cast<uint8_t>((x.g * multiplier.g * 258u) >> 16u),
            static_cast<uint8_t>((x.b * multiplier.b * 258u) >> 16u),
            static_cast<uint8_t>((x.a * multiplier.a * 258u) >> 16u)
    };
}

inline Rgba& operator*=(Rgba& x, Rgba multiplier) {
    x = x * multiplier;
    return x;
}

Rgba toRGBA(const Color& color) {
    const float k = 1.0f / 255.0f;
    return Rgba{
            (uint8_t) (k * color.r),
            (uint8_t) (k * color.g),
            (uint8_t) (k * color.b),
            (uint8_t) (k * color.a)
    };
}

Rgba toRGBA(uint32_t value) {
    return *(Rgba*) &value;
}

Color toColor(Rgba rgba) {
    const float k = 1.0f / 255.0f;
    return Color{
            k * (float) rgba.r,
            k * (float) rgba.g,
            k * (float) rgba.b,
            k * (float) rgba.a
    };
}

uint32_t toU32(Rgba rgba) {
    return *(uint32_t*) &rgba;
}

float saturate(float v) {
    return v < 0.0f ? 0.0f : (v > 1.0f ? 1.0f : v);
}

union PixelsData {
    Rgba* rgba;
    uint32_t* u32;
    void* data;
};

class Bitmap {
public:
    uint32_t* data = nullptr;
    int w = 0;
    int h = 0;

    Bitmap();

    Bitmap(int width, int height);

    ~Bitmap();

    [[nodiscard]]
    const Rgba* row(int y) const;

    Rgba* row(int y);

    void assign(const Bitmap& source);
};

void copyPixels(Bitmap& dest, int dx, int dy,
                const Bitmap& src, int sx, int sy, int sw, int sh);

void blit(Bitmap& dest, const Bitmap& src);

void convert_a8_to_argb32pma(uint8_t const* source_a8_buf, uint32_t* dest_argb32_buf, int pixels_count);

void convert_a8_to_argb32(uint8_t const* source_a8_buf,
                          uint32_t* dest_argb32_buf,
                          int pixels_count);

void undoPremultiplyAlpha(Bitmap& bitmap);
}
