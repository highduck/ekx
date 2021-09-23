#pragma once

#include "Bitmap.h"
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <vector>
#include <string>
#include <unordered_map>

#define BMFE_LOG_F(x, ...) printf(x, __VA_ARGS__)

namespace pugi {

class xml_node;

}

namespace bmfont_export {

struct RectI {
    int x;
    int y;
    int w;
    int h;
};

struct Rect {
    float x;
    float y;
    float w;
    float h;
};

struct Vec2 {
    float x;
    float y;
};

struct Vec2I {
    int x;
    int y;
};

enum class FilterType {
    None,
    ReFill,
    Glow,
    Shadow
};

struct Filter {
    FilterType type = FilterType::None;
    float blurX = 0.0f;
    float blurY = 0.0f;
    float strength = 1.0f;
    int quality = 1;
    Color color0{0, 0, 0, 1};
    Color color1{1, 1, 1, 1};

    // TODO: check we change it from radian to degree in serialization, we need change calculations for usages
    float angle = 0.0f;
    float distance = 0.0f;

    // refill options
    float top = 0.0f;
    float bottom = 100.0f;

    // flags
    bool knockout = false;
    bool innerGlow = false;
    bool hideObject = false;

    void readFromXML(const pugi::xml_node& node);
};

struct CodepointPair {
    uint32_t a;
    uint32_t b;
};

struct Image {
    std::string name;

    // source rectangle in base physical units
    Rect rc;

    RectI source;

    uint8_t padding = 1;

    // reference image;
    Bitmap* bitmap = nullptr;
};

struct Resolution {
    int index = 0;
    float scale = 1.0f;
    std::vector<Image> images{};
};

struct ImageCollection {
    std::vector<Resolution> resolutions;

    void addResolution(float scale) {
        Resolution res{};
        res.index = (int) resolutions.size();
        res.scale = scale;
        resolutions.push_back(res);
    }
};

struct BuildBitmapFontSettings {
    const char* name;
    const char* ttfPath;

    std::vector<CodepointPair> ranges;
    std::vector<Filter> filters;

    std::string atlas{"main"};
    std::vector<float> resolutions = {1.0f, 2.0f, 3.0f, 4.0f};

    uint16_t fontSize;

    bool mirrorCase = true;
    bool useKerning = true;

    void readFromXML(const pugi::xml_node& node);
};

//// exported STRUCTS

class Glyph {
public:
    std::vector<uint32_t> codepoints;
    RectI box;
    int32_t advanceWidth;
    std::string sprite;
};

class Font {
public:
    // 16 bits, but we need align memory for reading buffer
    int32_t unitsPerEM;
    int32_t fontSize;
    int32_t lineHeight;
    int32_t ascender;
    int32_t descender;
    std::vector<Glyph> glyphs;
};

/// filters functions
RectI get_filtered_rect(const RectI& rc, const std::vector<Filter>& filters);

std::vector<Filter> apply_scale(const std::vector<Filter>& filters, float scale);

void apply(Bitmap& image, const Filter& filter, const RectI& bounds);

void apply(const std::vector<Filter>& filters, Image& image, float scale);


/// font export main

Font buildBitmapFont(const BuildBitmapFontSettings& decl,
                     ImageCollection& imageCollection);

}