#pragma once

#include <string>
#include <vector>
#include <ek/math/box.hpp>
#include <ek/math/color_transform.hpp>
#include <ek/math/mat3x2.hpp>

#include "graphic_types.h"
#include "bitmap.h"
#include "filter.h"
#include "edge.h"
#include "blend_mode.h"
#include "motion.h"
#include "text.h"

namespace ek::flash {

struct element_t;

enum class tween_target : uint8_t {
    all = 0,
    position = 1,
    rotation = 2,
    scale = 3,
    color = 4,
    filters = 5
};

struct tween_object_t {
    tween_target target = tween_target::all;
    int intensity; // <Ease intensity="-100...100" />
    std::vector<float2> custom_ease;
};

struct frame_t {
    int index = 0;
    int duration = 1;
    tween_type tweenType = tween_type::none;
    int keyMode = 0;

    bool motionTweenSnap;
    bool motionTweenOrientToPath;
    rotation_direction motionTweenRotate = rotation_direction::none;
    int motionTweenRotateTimes = 0;

    const char* name; // label
    int acceleration; // ease -100...100

    bool hasCustomEase;
    std::vector<tween_object_t> tweens;

    std::string script;
    std::vector<element_t> elements;
    motion_object_t motionObject;
};

enum class layer_type {
    normal,
    guide
};

struct layer_t {
    std::string name;
    layer_type layerType = layer_type::normal;
    float4 color;

    bool autoNamed = false;
    bool current = false;
    bool isSelected = false;
    bool locked = false;

    std::vector<frame_t> frames;

    size_t duration() const {
        int total = 0;
        for (const auto& frame : frames) {
            total += frame.duration;
        }
        return total;
    }
};

struct timeline_t {
    std::string name;
    std::vector<layer_t> layers;

    size_t getTotalFrames() const {
        size_t res = 1u;
        for (const auto& layer : layers) {
            res = std::max(layer.duration(), res);
        }
        return res;
    }
};

enum class element_type {
    unknown,
    shape,
    group,
    dynamic_text,
    static_text,
    symbol_instance,
    bitmap_instance,
    symbol_item,
    bitmap_item,
    font_item,
    sound_item
};

enum class symbol_type {
    normal,
    button,
    graphic
};

struct item_properties {
    std::string name;
    std::string itemID;

    int lastModified;
    int sourceLastImported;
    std::string sourceExternalFilepath;

    std::string linkageClassName; //="Font1"
    std::string linkageBaseClass; //="flash.text.Font"
    bool linkageExportForAS;
};

struct element_t {
    item_properties item;

    element_type elementType = element_type::unknown;

    /** Transform point (Free Transform Tool) for current element, in LOCAL SPACE (do not applicate matrix) **/
    float2 transformationPoint;
    matrix_2d matrix;
    color_transform_f color;
    rect_f rect;

    /// SYMBOL ITEM
    timeline_t timeline{};

    rect_f scaleGrid; //scaleGridLeft="-2" scaleGridRight="2" scaleGridTop="-2" scaleGridBottom="2"

/// ref to item
    std::string libraryItemName;

////// group
    std::vector<element_t> members;

///// SHAPE
    std::vector<edge_t> edges;
    std::vector<fill_style> fills;
    std::vector<stroke_style> strokes;
    bool isDrawingObject;

    ///// Symbol instance
    symbol_type symbolType;
    float centerPoint3DX;
    float centerPoint3DY;
    bool cacheAsBitmap;
    bool exportAsBitmap;
    const char* loop;
    bool silent = false;
    bool forceSimple = false;
    bool isVisible = true;

    //// text
    bool isSelectable;
    std::vector<text_run_t> textRuns;
    std::vector<filter_t> filters;

    // dynamic text
    bool border;// = false;
    const char* fontRenderingMode;
    bool autoExpand;// = false;
    const char* lineType; // lineType="multiline no wrap"

    blend_mode_t blend_mode = blend_mode_t::last;

    // bitmap item
    std::string bitmapDataHRef;
    //public var frameBottom:Int;
    //public var frameRight:Int;

    std::string href;
    bool isJPEG;
    int quality;

    std::unique_ptr<bitmap_t> bitmap = nullptr;

    /// FONT ITEM
    std::string font;
    int size = 0;
    int id = 0;
    // embedRanges="1|2|3|4|5"

    // SOUND ITEM
    // sourcePlatform="macintosh"
    // externalFileSize="18807"
    // href="media_test/next_level.mp3"
    // soundDataHRef="M 23 1553173508.dat"
    // format="44kHz 16bit Stereo"
    // sampleCount="51840"
    // exportFormat="1"
    // exportBits="7"
    // dataLength="18807"
    // exportNative="true"
    // cacheFormat="5kHz 8bit Stereo"
    // cachedSampleCount="6480"

    element_t() = default;
};

struct folder_item {
    item_properties item;
    bool isExpanded = false;
};

struct document_info {
    int width = 550;
    int height = 400;
    float xflVersion = 0.0f;
    int frameRate = 24;
    int backgroundColor = 0;
    int buildNumber = 0;
    int currentTimeline = 0;
    int majorVersion = 0;
    float viewAngle3D = 0.0f;
};

}


