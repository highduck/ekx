#pragma once

#include <string>
#include <vector>
#include <optional>
#include <ek/math/box.hpp>
#include <ek/math/color_transform.hpp>
#include <ek/math/mat3x2.hpp>
#include <ek/util/path.hpp>

namespace ek::xfl {

struct BitmapData {
    //std::string path;
    int width = 0;
    int height = 0;
    int bpp = 4;
    bool alpha = true;
    std::vector<uint8_t> data;
};

enum class FilterType {
    none = 0,
    drop_shadow = 1,
    glow = 2,
    blur = 3,
    bevel = 4,
    convolution = 5,
    adjust_color = 6
};

struct Filter {
    FilterType type = FilterType::none;

    float4 color;
    float2 blur;
    float distance = 0.0f;
    float angle = 0.0f; // degrees
    uint8_t quality = 1; // TODO: check
    float strength = 1.0f;
    bool inner = false;
    bool knockout = false;
    bool hideObject = false;
};


enum class blend_mode_t {
    last, // default
    normal, // default
    layer,
    multiply,
    screen,
    lighten,
    darken,
    difference,
    add,
    subtract,
    invert,
    alpha,
    erase,
    overlay,
    hardlight
};

enum class FontRenderingMode {
    // antialias for readability (default, value not known)
    normal,
    // antialias for animation
    standard,
    //
    customThicknessSharpness,
    device,
    bitmap
};

enum class TextLineType {
    // default, value not known
    SingleLine,
    Multiline,
    MultilineNoWrap
};

struct edge_t {
    std::vector<char> commands;
    std::vector<double> values;
    int fill_style_0 = 0;
    int fill_style_1 = 0;
    int stroke_style = 0;
};

struct text_attributes_t {
    float2 alignment{};// alignment = "left"; / center / right
    bool aliasText = false;
    bool bold = false;
    bool italic = false;
    float4 color{0.0f, 0.0f, 0.0f, 1.0f};
    std::string face; // face="Font 1*"
    float lineHeight = 20; // 20
    float lineSpacing = 0; // "-14";
    float size = 32;// = "32";
    uint32_t bitmapSize = 640; // just twips size
    bool autoKern = true;
};

struct text_run_t {
    std::string characters;
    text_attributes_t attributes;
};


struct motion_object_t {
    int duration;
    int timeScale;
};

enum class tween_type {
    none,
    classic,
    motion_object
};

enum class rotation_direction {
    none = 0,
    ccw,
    cw
};


enum class scale_mode {
    none,
    normal,
    horizontal,
    vertical
};

enum class solid_style_type {
    hairline
};

enum class line_caps {
    none,
    round, // default
    square
};

enum class line_joints {
    miter,
    round, // default
    bevel
};

enum class fill_type {
    unknown = 0,
    solid = 1,
    linear = 2,
    radial = 3,
    bitmap = 4
};

enum class spread_method {
    extend = 0,
    reflect = 1,
    repeat = 2
};

struct gradient_entry {
    float4 color;
    float ratio = 0.0f;

    gradient_entry() = default;

    explicit gradient_entry(const float4& color, float ratio = 0.0f)
            : color{color},
              ratio{ratio} {
    }
};

struct fill_style {
    int index = 0;

    fill_type type = fill_type::solid;
    spread_method spreadMethod = spread_method::repeat;
    std::vector<gradient_entry> entries;
    matrix_2d matrix{};
    std::string bitmapPath;
    std::shared_ptr<BitmapData> bitmap;
};

struct stroke_style {
    int index = 0;
    bool is_solid = true;

    fill_style fill{};
    scale_mode scaleMode = scale_mode::none;
    solid_style_type solidStyle = solid_style_type::hairline;
    float weight = 1.0f;
    line_caps caps = line_caps::round;
    line_joints joints = line_joints::round;
    float miterLimit = 0.0f;
    bool pixelHinting = false;
};

/**** elements tree ****/


struct element_t;

enum class tween_target {
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

    [[nodiscard]]
    int endFrame() const {
        return index + duration - 1;
    }

    int duration = 1;
    tween_type tweenType = tween_type::none;
    int keyMode = 0;

    bool motionTweenSnap;
    bool motionTweenOrientToPath;
    rotation_direction motionTweenRotate = rotation_direction::none;
    int motionTweenRotateTimes = 0;

    std::string name; // label
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

    [[nodiscard]]
    size_t duration() const {
        int total = 0;
        for (const auto& frame : frames) {
            total += frame.duration;
        }
        return total;
    }

    [[nodiscard]]
    size_t getElementsCount() const {
        int i = 0;
        for (auto& frame : frames) {
            i += frame.elements.size();
        }
        return i;
    }
};

struct timeline_t {
    std::string name;
    std::vector<layer_t> layers;

    [[nodiscard]]
    size_t getTotalFrames() const {
        size_t res = 1u;
        for (const auto& layer : layers) {
            res = std::max(layer.duration(), res);
        }
        return res;
    }

    [[nodiscard]]
    size_t getElementsCount() const {
        size_t i = 0;
        for (auto& layer : layers) {
            i += layer.getElementsCount();
        }
        return i;
    }
};

struct shape_object_t {
    float objectWidth = 0;
    float objectHeight = 0;
    float x = 0;
    float y = 0;
//    fill?: DOMFillStyle;
//    stroke?: DOMStrokeStyle;

    // Oval
    float endAngle = 0;
    float startAngle = 0;
    float innerRadius = 0;
    bool closePath = true;

    // Rectangle
    float topLeftRadius = 0;
    float topRightRadius = 0;
    float bottomLeftRadius = 0;
    float bottomRightRadius = 0;
    bool lockFlag = false;
};

enum class element_type {
    unknown,
    shape,
    object_oval,
    object_rectangle,
    group,
    dynamic_text,
    static_text,
    symbol_instance,
    bitmap_instance,
    symbol_item,
    bitmap_item,
    font_item,
    sound_item,

    scene_timeline
};

enum class symbol_type {
    normal,
    button,
    graphic
};

enum class loop_mode {
    none = 0,
    loop,
    play_once,
    single_frame
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

    loop_mode loop;
    int firstFrame;

    bool silent = false;
    bool forceSimple = false;
    bool isVisible = true;

    //// text
    std::vector<text_run_t> textRuns;
    std::vector<Filter> filters;

    // dynamic text
    bool isSelectable = true;
    bool border = false;
    bool autoExpand = false;
    bool scrollable = false;
    bool renderAsHTML = false;
    FontRenderingMode fontRenderingMode = FontRenderingMode::normal;
    TextLineType lineType = TextLineType::SingleLine;

    blend_mode_t blend_mode = blend_mode_t::last;

    // bitmap item
    std::string bitmapDataHRef;
    //public var frameBottom:Int;
    //public var frameRight:Int;

    std::string href;
    bool isJPEG;
    int quality;

    std::shared_ptr<BitmapData> bitmap = nullptr;

    /// FONT ITEM
    std::string font;
    int size = 0;
    int id = 0;
    // embedRanges="1|2|3|4|5"

    std::optional<shape_object_t> shape;

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