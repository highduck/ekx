#pragma once

#include <string>
#include <vector>
#include <optional>
#include <ek/math/serialize_math.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/math/box.hpp>
#include <ek/math/color_transform.hpp>
#include <ek/util/Res.hpp>
#include <ek/scenex/text/TextFormat.hpp>
#include <ecxx/ecxx.hpp>

namespace ek {

enum class SGFilterType {
    None,
    DropShadow,
    Glow
};

struct SGFilter {
    SGFilterType type = SGFilterType::None;
    uint32_t quality = 1;
    argb32_t color = argb32_t::one;
    float2 blur;
    float2 offset;

    template<typename S>
    void serialize(IO<S>& io) {
        io(type, quality, color, blur, offset);
    }
};

struct SGTextLayerData {
    argb32_t color = 0xFFFFFFFF_argb;
    float2 offset{};
    float blurRadius = 0.0f;
    int blurIterations = 0;
    int strength = 0;

    template<typename S>
    void serialize(IO<S>& io) {
        io(color, offset, blurRadius, blurIterations, strength);
    }
};

struct SGDynamicTextData {
    std::string text;
    std::string font;
    float size;
    float2 alignment;
    rect_f rect;
    float lineSpacing = 0.0f;
    float lineHeight = 0.0f;

    std::vector<SGTextLayerData> layers;

    bool wordWrap = false;

    template<typename S>
    void serialize(IO<S>& io) {
        io(text, font, size, alignment, rect, lineSpacing, lineHeight, layers, wordWrap);
    }
};

// TODO: it should be optimized with easing table store :)
struct SGEasingData {
    uint8_t attribute = 0;
    float ease = 0.0f;
    std::vector<float2> curve;

    template<typename S>
    void serialize(IO<S>& io) {
        io(attribute, ease, curve);
    }
};

struct SGKeyFrameTransform {
    float2 position;
    float2 scale{1.0f, 1.0f};
    float2 skew;
    float2 pivot;
    color_transform_f color;

    template<typename S>
    void serialize(IO<S>& io) {
        io(position, scale, skew, pivot, color);
    }

    SGKeyFrameTransform operator-(const SGKeyFrameTransform& v) const {
        return {
                position - v.position,
                scale - v.scale,
                skew - v.skew,
                pivot - v.pivot,
                color - v.color,
        };
    }

    SGKeyFrameTransform operator+(const SGKeyFrameTransform& v) const {
        return {
                position + v.position,
                scale + v.scale,
                skew + v.skew,
                pivot + v.pivot,
                color + v.color,
        };
    }
};

struct SGMovieFrameData {
    int index = 0;
    int duration = 0;
    int motion_type = 0;

    std::vector<SGEasingData> easing;

    SGKeyFrameTransform transform;

    bool visible = true;

    // graphic frame control
    int loopMode = 0; // loop_mode
    int firstFrame = 0;

    // rotation postprocessing

    int rotate = 0; // rotate_direction
    int rotateTimes = 0;

    template<typename S>
    void serialize(IO<S>& io) {
        io(index, duration, motion_type, transform, easing,
           loopMode, firstFrame, rotate, rotateTimes, visible);
    }

    [[nodiscard]]
    float getLocalTime(float time) const {
        return (time - static_cast<float>(index)) / static_cast<float>(duration);
    }
};

struct SGNodeData;

struct SGMovieLayerData {
    std::vector<SGMovieFrameData> frames;

    // temp for restoring target ID
    std::vector<SGNodeData*> targets;

    template<typename S>
    void serialize(IO<S>& io) {
        io(frames);
    }
};

struct SGMovieData {
    int frames = 1;
    float fps = 24.0f;
    std::vector<SGMovieLayerData> layers;

    template<typename S>
    void serialize(IO<S>& io) {
        io(frames, fps, layers);
    }
};

struct SGNodeData {

    matrix_2d matrix{};
    color_transform_f color{};

    // instance name
    std::string name;

    // name in library
    std::string libraryName;

    // sprite id
    std::string sprite;

    bool button = false;
    bool touchable = true;
    bool visible = true;
    bool scissorsEnabled = false;
    bool hitAreaEnabled = false;
    bool boundsEnabled = false;
    rect_f boundingRect;
    rect_f scaleGrid;
    std::vector<SGNodeData> children;
    std::vector<SGFilter> filters;
    std::optional<SGDynamicTextData> dynamicText;
    std::optional<SGMovieData> movie;
    int32_t movieTargetId = -1;

    std::unordered_map<int, std::string> labels;
    std::unordered_map<int, std::string> scripts;

    template<typename S>
    void serialize(IO<S>& io) {
        io(
                name,
                libraryName,
                matrix,
                sprite,

                button,
                touchable,
                visible,
                scissorsEnabled,
                hitAreaEnabled,
                boundsEnabled,

                boundingRect,
                scaleGrid,

                // TODO: labels and scripts
                //script,
                color,

                children,
                filters,
                dynamicText,
                movie,

                movieTargetId
        );
    }
};

struct SGFile {
    std::vector<std::string> scenes;
    std::unordered_map<std::string, std::string> linkages;
    std::vector<SGNodeData> library;

    template<typename S>
    void serialize(IO<S>& io) {
        io(scenes, linkages, library);
    }

    [[nodiscard]]
    const SGNodeData* get(const std::string& libraryName) const;
};

SGFile* sg_load(const std::vector<uint8_t>& buffer);

ecs::entity sg_create(const std::string& library, const std::string& name);

rect_f sg_get_bounds(const std::string& library, const std::string& name);

}


