#pragma once

#include <ek/log.h>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>
#include <ek/util/Type.hpp>
#include <ek/math/MathSerialize.hpp>
#include <ek/math/Color32.hpp>


#include <ek/serialize/serialize.hpp>

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
    vec2_t blur;
    vec2_t offset;

    template<typename S>
    void serialize(IO<S>& io) {
        io(type, quality, color, blur, offset);
    }
};

struct SGTextLayerData {
    argb32_t color = 0xFFFFFFFF_argb;
    vec2_t offset{};
    float blurRadius = 0.0f;
    int blurIterations = 0;
    int strength = 0;

    template<typename S>
    void serialize(IO<S>& io) {
        io(color, offset, blurRadius, blurIterations, strength);
    }
};

struct SGDynamicTextData {
    String text;
    String font;
    float size;
    vec2_t alignment;
    rect_t rect;
    float lineSpacing = 0.0f;
    float lineHeight = 0.0f;

    Array<SGTextLayerData> layers;

    bool wordWrap = false;

    template<typename S>
    void serialize(IO<S>& io) {
        io(text, font, size, alignment, rect, lineSpacing, lineHeight, layers, wordWrap);
        //log_info("layers size: %u", layers.size());
    }
};

// TODO: it should be optimized with easing table store :)
struct SGEasingData {
    uint8_t attribute = 0;
    float ease = 0.0f;
    Array<vec2_t> curve{};

    template<typename S>
    void serialize(IO<S>& io) {
        io(attribute, ease, curve);
    }
};

struct SGKeyFrameTransform {
    vec2_t position;
    vec2_t scale = {{1.0f, 1.0f}};
    vec2_t skew;
    vec2_t pivot;
    color2f_t color = color2f();

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

    Array<SGEasingData> easing{};

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
    Array<SGMovieFrameData> frames;

    // temp for restoring target ID
    Array<SGNodeData*> targets;

    template<typename S>
    void serialize(IO<S>& io) {
        io(frames);
    }
};

struct SGFrameLabel {
    String name;
    int frame;
};

struct SGFrameScript {
    String code;
    int frame;
};

struct SGMovieData {
    int frames = 1;
    float fps = 24.0f;
    Array<SGMovieLayerData> layers;

    template<typename S>
    void serialize(IO<S>& io) {
        // TODO: labels and scripts
        io(frames, fps, layers);
    }
};

struct SGNodeData {

    mat3x2_t matrix = mat3x2_identity();
    color2f_t color = color2f();

    // instance name
    String name;

    // name in library
    String libraryName;

    // sprite id
    String sprite;

    bool button = false;
    bool touchable = true;
    bool visible = true;
    bool scissorsEnabled = false;
    bool hitAreaEnabled = false;
    bool boundsEnabled = false;
    rect_t boundingRect;
    rect_t scaleGrid;
    Array<SGNodeData> children;
    Array<SGFilter> filters;
    Array<SGDynamicTextData> dynamicText;
    Array<SGMovieData> movie;
    Array<SGFrameLabel> labels;
    Array<SGFrameScript> scripts;
    int32_t movieTargetId = -1;

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

                color,

                children,
                filters,
                dynamicText,
                movie,

                movieTargetId
        );
    }
};

struct SGSceneInfo {
    String name;
    String linkage;

    template<typename S>
    void serialize(IO<S>& io) {
        io(name, linkage);
    }
};

class SGFile {
public:
    Array<String> scenes;
    Array<SGSceneInfo> linkages;
    Array<SGNodeData> library;

    template<typename S>
    void serialize(IO<S>& io) {
        io(scenes, linkages, library);
    }
};

//EK_DECLARE_TYPE(SGFile);
EK_TYPE_INDEX(SGFile, 11);

}

