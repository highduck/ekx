#pragma once

#include <string>
#include <ek/math.h>
#include <ek/ds/Array.hpp>
#include <memory>
#include <optional>

#include <ek/util/Path.hpp>

namespace ek::xfl {

struct BitmapData {
    int width = 0;
    int height = 0;
    int bpp = 4;
    bool alpha = true;
    Array<uint8_t> data;

    // TODO: data is data, remove string usage
    static BitmapData* parse(const void* data, uint32_t size);
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

    vec4_t color = {};
    vec2_t blur = {};
    float distance = 0.0f;
    float angle = 0.0f; // degrees
    uint8_t quality = 1; // TODO: check
    float strength = 1.0f;
    bool inner = false;
    bool knockout = false;
    bool hideObject = false;
};

enum class BlendMode {
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

struct Edge {
    Array<char> commands;
    Array<double> values;
    int fill_style_0 = 0;
    int fill_style_1 = 0;
    int stroke_style = 0;
};

struct TextAttributes {
    vec2_t alignment = {};// alignment = "left"; / center / right
    bool aliasText = false;
    bool bold = false;
    bool italic = false;
    vec4_t color = {{0, 0, 0, 1}};
    String face; // face="Font 1*"
    float lineHeight = 20; // 20
    float lineSpacing = 0; // "-14";
    float size = 32;// = "32";
    uint32_t bitmapSize = 640; // just twips size
    bool autoKern = true;
};

struct TextRun {
    String characters;
    TextAttributes attributes;
};

struct MotionObject {
    int duration;
    int timeScale;
};

enum class TweenType {
    none,
    classic,
    motion_object
};

enum class RotationDirection {
    none = 0,
    ccw,
    cw
};

enum class ScaleMode {
    none,
    normal,
    horizontal,
    vertical
};

enum class SolidStyleType {
    hairline
};

enum class LineCaps {
    none,
    round, // default
    square
};

enum class LineJoints {
    miter,
    round, // default
    bevel
};

enum class FillType {
    unknown = 0,
    solid = 1,
    linear = 2,
    radial = 3,
    bitmap = 4
};

enum class SpreadMethod {
    extend = 0,
    reflect = 1,
    repeat = 2
};

struct GradientEntry {
    vec4_t color = {};
    float ratio = 0.0f;

    GradientEntry() = default;

    explicit GradientEntry(const vec4_t& color, float ratio = 0.0f) : color{color},
                                                                     ratio{ratio} {
    }
};

struct FillStyle {
    int index = 0;

    FillType type = FillType::solid;
    SpreadMethod spreadMethod = SpreadMethod::repeat;
    Array<GradientEntry> entries{};
    mat3x2_t matrix = mat3x2_identity();
    String bitmapPath;
    std::shared_ptr<BitmapData> bitmap;
};

struct StrokeStyle {
    int index = 0;
    bool is_solid = true;

    FillStyle fill{};
    ScaleMode scaleMode = ScaleMode::none;
    SolidStyleType solidStyle = SolidStyleType::hairline;
    float weight = 1.0f;
    LineCaps caps = LineCaps::round;
    LineJoints joints = LineJoints::round;
    float miterLimit = 0.0f;
    bool pixelHinting = false;
};

/**** elements tree ****/


struct Element;

enum class TweenTarget {
    all = 0,
    position = 1,
    rotation = 2,
    scale = 3,
    color = 4,
    filters = 5
};

struct TweenObject {
    TweenTarget target = TweenTarget::all;
    int intensity; // <Ease intensity="-100...100" />
    Array<vec2_t> custom_ease;
};

struct Frame {
    int index = 0;

    [[nodiscard]] int endFrame() const {
        return index + duration - 1;
    }

    int duration = 1;
    TweenType tweenType = TweenType::none;
    int keyMode = 0;

    bool motionTweenSnap;
    bool motionTweenOrientToPath;
    RotationDirection motionTweenRotate = RotationDirection::none;
    int motionTweenRotateTimes = 0;

    String name; // label
    int acceleration; // ease -100...100

    bool hasCustomEase;
    Array<TweenObject> tweens{};

    String script;
    Array<Element> elements{};
    MotionObject motionObject;
};

enum class LayerType {
    normal,
    guide
};

struct Layer {
    String name;
    LayerType layerType = LayerType::normal;
    vec4_t color = {};

    bool autoNamed = false;
    bool current = false;
    bool isSelected = false;
    bool locked = false;

    Array<Frame> frames{};

    [[nodiscard]]
    int duration() const {
        int total = 0;
        for (const auto& frame : frames) {
            total += frame.duration;
        }
        return total;
    }

    [[nodiscard]]
    int getElementsCount() const {
        int i = 0;
        for (auto& frame : frames) {
            i += frame.elements.size();
        }
        return i;
    }
};

struct Timeline {
    String name;
    Array<Layer> layers{};

    [[nodiscard]]
    int getTotalFrames() const {
        int res = 1u;
        for (const auto& layer : layers) {
            res = std::max(layer.duration(), res);
        }
        return res;
    }

    [[nodiscard]]
    int getElementsCount() const {
        int i = 0;
        for (auto& layer : layers) {
            i += layer.getElementsCount();
        }
        return i;
    }
};

struct ShapeObject {
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

enum class ElementType {
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

enum class SymbolType {
    normal,
    button,
    graphic
};

enum class LoopMode {
    none = 0,
    loop,
    play_once,
    single_frame
};

struct ItemProperties {
    String name;
    String itemID;

    int lastModified;
    int sourceLastImported;
    String sourceExternalFilepath;

    String linkageClassName; //="Font1"
    String linkageBaseClass; //="flash.text.Font"
    bool linkageExportForAS;
};

struct TransformModel {
    mat3x2_t matrix = mat3x2_identity();
    color2f_t color = color2f();
    BlendMode blendMode = BlendMode::normal;

    TransformModel() = default;

    TransformModel(const mat3x2_t matrix_, const color2f_t color_, BlendMode blend_mode_ = BlendMode::last) :
            matrix{matrix_},
            color{color_},
            blendMode{blend_mode_} {
    }

    inline TransformModel operator*(const TransformModel& right) const {
        return {
                mat3x2_mul(matrix, right.matrix),
                color * right.color,
                right.blendMode == BlendMode::last ? blendMode : right.blendMode
        };
    }
};

struct Element {
    ItemProperties item;

    ElementType elementType = ElementType::unknown;

    /** Transform point (Free Transform Tool) for current element, in LOCAL SPACE (do not applicate matrix) **/
    TransformModel transform;
    vec2_t transformationPoint = {};
    rect_t rect = {};

    /// SYMBOL ITEM
    Timeline timeline{};

    rect_t scaleGrid = {}; //scaleGridLeft="-2" scaleGridRight="2" scaleGridTop="-2" scaleGridBottom="2"

/// ref to item
    String libraryItemName;

////// group
    Array<Element> members{};

///// SHAPE
    Array<Edge> edges{};
    Array<FillStyle> fills{};
    Array<StrokeStyle> strokes{};
    bool isDrawingObject;

    ///// Symbol instance
    SymbolType symbolType;
    float centerPoint3DX;
    float centerPoint3DY;
    bool cacheAsBitmap;
    bool exportAsBitmap;

    LoopMode loop;
    int firstFrame;

    bool silent = false;
    bool forceSimple = false;
    bool isVisible = true;

    //// text
    Array<TextRun> textRuns{};
    Array<Filter> filters{};

    // dynamic text
    bool isSelectable = true;
    bool border = false;
    bool autoExpand = false;
    bool scrollable = false;
    bool renderAsHTML = false;
    FontRenderingMode fontRenderingMode = FontRenderingMode::normal;
    TextLineType lineType = TextLineType::SingleLine;

    // bitmap item
    String bitmapDataHRef;
    //public var frameBottom:Int;
    //public var frameRight:Int;

    String href;
    bool isJPEG;
    int quality;

    std::shared_ptr<BitmapData> bitmap = nullptr;

    /// FONT ITEM
    String font;
    int size = 0;
    int id = 0;
    // embedRanges="1|2|3|4|5"

    std::optional<ShapeObject> shape;

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

    Element() = default;
};

struct FolderItem {
    ItemProperties item;
    bool isExpanded = false;
};

struct DocInfo {
    int width = 550;
    int height = 400;
    float xflVersion = 0.0f;
    int frameRate = 24;
    uint32_t backgroundColor = 0u;
    int buildNumber = 0;
    int currentTimeline = 0;
    int majorVersion = 0;
    float viewAngle3D = 0.0f;
};

}