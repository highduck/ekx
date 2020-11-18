#pragma once

#include <string>
#include <vector>
#include <optional>
#include <ek/math/serialize_math.hpp>
#include <ek/math/packed_color.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/math/box.hpp>
#include <ek/math/color_transform.hpp>
#include <ek/util/assets.hpp>
#include <ek/scenex/text/TextFormat.hpp>

namespace ek {

enum class sg_filter_type {
    none,
    drop_shadow,
    glow
};

struct filter_data {
    sg_filter_type type = sg_filter_type::none;
    uint32_t quality = 1;
    argb32_t color;
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

struct dynamic_text_data {
    std::string text;
    std::string font;
    float size;
    float2 alignment;
    rect_f rect;
    float line_spacing = 0.0f;
    float line_height = 0.0f;

    std::vector<SGTextLayerData> layers;

    template<typename S>
    void serialize(IO<S>& io) {
        io(text, font, size, alignment, rect, line_spacing, line_height, layers);
    }
};

// TODO: it should be optimized with easing table store :)
struct easing_data_t {
    uint8_t attribute = 0;
    float ease = 0.0f;
    std::vector<float2> curve;

    template<typename S>
    void serialize(IO<S>& io) {
        io(attribute, ease, curve);
    }
};

struct keyframe_transform_t {
    float2 position;
    float2 scale{1.0f, 1.0f};
    float2 skew;
    float2 pivot;
    color_transform_f color;

    template<typename S>
    void serialize(IO<S>& io) {
        io(position, scale, skew, pivot, color);
    }

    keyframe_transform_t operator-(const keyframe_transform_t& v) const {
        return {
                position - v.position,
                scale - v.scale,
                skew - v.skew,
                pivot - v.pivot,
                color - v.color,
        };
    }

    keyframe_transform_t operator+(const keyframe_transform_t& v) const {
        return {
                position + v.position,
                scale + v.scale,
                skew + v.skew,
                pivot + v.pivot,
                color + v.color,
        };
    }
};

struct movie_frame_data {
    int index = 0;
    int duration = 0;
    int motion_type = 0;

    std::vector<easing_data_t> easing;

    keyframe_transform_t transform;

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

struct sg_node_data;

struct movie_layer_data {
    std::vector<movie_frame_data> frames;

    // temp for restoring target ID
    std::vector<sg_node_data*> targets;

    template<typename S>
    void serialize(IO<S>& io) {
        io(frames);
    }
};

struct sg_movie_data {
    int frames = 1;
    float fps = 24.0f;
    std::vector<movie_layer_data> layers;

    template<typename S>
    void serialize(IO<S>& io) {
        io(frames, fps, layers);
    }
};

struct sg_node_data {

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
    std::vector<sg_node_data> children;
    std::vector<filter_data> filters;
    std::optional<dynamic_text_data> dynamicText;
    std::optional<sg_movie_data> movie;
    int movieTargetId = -1;

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

struct sg_file {
    std::vector<std::string> scenes;
    std::unordered_map<std::string, std::string> linkages;
    std::vector<sg_node_data> library;

    template<typename S>
    void serialize(IO<S>& io) {
        io(scenes, linkages, library);
    }

    [[nodiscard]]
    const sg_node_data* get(const std::string& library_name) const;
};

}


