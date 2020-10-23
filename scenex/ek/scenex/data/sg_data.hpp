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

struct dynamic_text_data {
    rect_f rect;
    std::string text;
    std::string face;
    float2 alignment;
    float line_spacing = 0.0f;
    float line_height = 0.0f;
    float size;
    argb32_t color;

    template<typename S>
    void serialize(IO<S>& io) {
        io(rect, text, face, alignment, line_spacing, line_height, size, color);
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

struct movie_frame_data {
    int index = 0;
    int duration = 0;
    int motion_type = 0;

    std::vector<easing_data_t> easing;

    float2 position;
    float2 scale;
    float2 skew;
    float2 pivot;
    color_transform_f color{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f};

    bool visible = true;

    // graphic frame control
    int loopMode = 0; // loop_mode
    int firstFrame = 0;

    // rotation postprocessing

    int rotate = 0; // rotate_direction
    int rotateTimes = 0;

    template<typename S>
    void serialize(IO<S>& io) {
        io(index, duration, motion_type, position, scale, skew, pivot, color, easing,
        loopMode, firstFrame, rotate, rotateTimes, visible);
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
    rect_f scaleGrid;
    rect_f hitRect;
    rect_f clipRect;
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

                // TODO: labels and scripts
                //script,
                color,
                scaleGrid,
                hitRect,
                clipRect,

                children,
                filters,
                dynamicText,
                movie,

                movieTargetId
        );
    }
};

struct sg_file {
    std::unordered_map<std::string, std::string> linkages;
    sg_node_data library;

    template<typename S>
    void serialize(IO<S>& io) {
        io(linkages, library);
    }

    [[nodiscard]]
    const sg_node_data* get(const std::string& library_name) const;
};

}


