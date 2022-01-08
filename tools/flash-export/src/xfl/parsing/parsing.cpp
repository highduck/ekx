#include "parsing.hpp"

#include "../types.hpp"
#include <ek/log.h>
#include <ek/assert.h>

#include <pugixml.hpp>

#include <cstring>

namespace ek::xfl {

/** Basic Types parsing **/

inline bool equals(const char* a, const char* b) {
    return a && strcmp(a, b) == 0;
}

static uint32_t parse_css_color(const char* str) {
    uint32_t col = 0x0;
    if (str) {
        if (*str == '#') {
            ++str;
        }
        col = strtoul(str, nullptr, 16);
    }
    return col;
}

rect_t read_rect(const xml_node& node) {
    return {{
            node.attribute("left").as_float(),
            node.attribute("top").as_float(),
            node.attribute("width").as_float(),
            node.attribute("height").as_float()
    }};
}

static rect_t read_rect_bounds(const xml_node& node, const char* nn[4]) {
    return rect_minmax(
            {{node.attribute(nn[0]).as_float(), node.attribute(nn[1]).as_float()}},
            {{node.attribute(nn[2]).as_float(), node.attribute(nn[3]).as_float()}}
    );
}

rect_t read_scale_grid(const xml_node& node) {
    const char* tags[4] = {
            "scaleGridLeft",
            "scaleGridTop",
            "scaleGridRight",
            "scaleGridBottom"
    };
    return read_rect_bounds(node, tags);
}

vec2_t read_point(const xml_node& node) {
    return {{node.attribute("x").as_float(), node.attribute("y").as_float()}};
}

vec2_t read_transformation_point(const xml_node& node) {
    return read_point(node.child("transformationPoint").child("Point"));
}

mat3x2_t& operator<<(mat3x2_t& r, const xml_node& node) {
    const auto& m = node.child("matrix").child("Matrix");
    r.a = m.attribute("a").as_float(1.0f);
    r.b = m.attribute("b").as_float();
    r.c = m.attribute("c").as_float();
    r.d = m.attribute("d").as_float(1.0f);
    r.tx = m.attribute("tx").as_float();
    r.ty = m.attribute("ty").as_float();
    return r;
}

ColorTransformF& operator<<(ColorTransformF& color, const xml_node& node) {
    const auto& ct = node.child("color").child("Color");

    color.scale.x = ct.attribute("redMultiplier").as_float(1.0f);
    color.scale.y = ct.attribute("greenMultiplier").as_float(1.0f);
    color.scale.z = ct.attribute("blueMultiplier").as_float(1.0f);
    color.scale.w = ct.attribute("alphaMultiplier").as_float(1.0f);

    color.offset.x = ct.attribute("redOffset").as_float() / 255.0f;
    color.offset.y = ct.attribute("greenOffset").as_float() / 255.0f;
    color.offset.z = ct.attribute("blueOffset").as_float() / 255.0f;
    color.offset.w = ct.attribute("alphaOffset").as_float() / 255.0f;

    auto tint_multiplier = ct.attribute("tintMultiplier").as_float();
    auto tint_color = parse_css_color(ct.attribute("tintColor").value());
    if (tint_multiplier > 0.0f) {
        color.tint(tint_color, tint_multiplier);
    }

    // default: 0, values: -1 ... 1
    float br = ct.attribute("brightness").as_float(0.0f);
    br = clamp(br, -1, 1);
    if (br < 0) {
        color.scale.x =
        color.scale.y =
        color.scale.z = 1 + br;
    } else if (br > 0.0f) {
        color.offset.x =
        color.offset.y =
        color.offset.z = br;
    }

    return color;
}

static vec4_t read_color(const xml_node& node, const char* color_tag = "color", const char* alpha_tag = "alpha") {
    const auto c = parse_css_color(node.attribute(color_tag).value());
    return {{
                    (float) ((c >> 16u) & 0xFFu) / 255.0f,
                    (float) ((c >> 8u) & 0xFFu) / 255.0f,
                    (float) (c & 0xFFu) / 255.0f,
                    node.attribute(alpha_tag).as_float(1.0f)
            }};
}

/** Element parsing **/
LayerType& operator<<(LayerType& r, const char* str) {
    if (equals(str, "guide")) return r = LayerType::guide;
    return r = LayerType::normal;
}

TweenType& operator<<(TweenType& r, const char* str) {
    if (equals(str, "motion")) return r = TweenType::classic;
    if (equals(str, "motion object")) return r = TweenType::motion_object;
    return r = TweenType::none;
}

TweenTarget& operator<<(TweenTarget& r, const char* str) {
    if (equals(str, "all")) return r = TweenTarget::all;
    if (equals(str, "position")) return r = TweenTarget::position;
    if (equals(str, "rotation")) return r = TweenTarget::rotation;
    if (equals(str, "scale")) return r = TweenTarget::scale;
    if (equals(str, "color")) return r = TweenTarget::color;
    if (equals(str, "filters")) return r = TweenTarget::filters;
    return r = TweenTarget::all;
}

FontRenderingMode& operator<<(FontRenderingMode& r, const char* str) {
    if (equals(str, "standard")) return r = FontRenderingMode::standard;
    if (equals(str, "device")) return r = FontRenderingMode::device;
    if (equals(str, "bitmap")) return r = FontRenderingMode::bitmap;
    if (equals(str, "customThicknessSharpness")) return r = FontRenderingMode::customThicknessSharpness;
    return r = FontRenderingMode::normal;
}

TextLineType& operator<<(TextLineType& r, const char* str) {
    if (equals(str, "multiline no wrap")) return r = TextLineType::MultilineNoWrap;
    if (equals(str, "multiline")) return r = TextLineType::Multiline;
    return r = TextLineType::SingleLine;
}

RotationDirection& operator<<(RotationDirection& r, const char* str) {
    if (equals(str, "counter-clockwise")) return r = RotationDirection::ccw;
    else if (equals(str, "clockwise")) return r = RotationDirection::cw;
    return r = RotationDirection::none;
}

ElementType& operator<<(ElementType& r, const char* str) {
    if (equals(str, "DOMGroup")) return r = ElementType::group;
    if (equals(str, "DOMBitmapInstance")) return r = ElementType::bitmap_instance;
    if (equals(str, "DOMBitmapItem")) return r = ElementType::bitmap_item;
    if (equals(str, "DOMShape")) return r = ElementType::shape;
    if (equals(str, "DOMOvalObject")) return r = ElementType::object_oval;
    if (equals(str, "DOMRectangleObject")) return r = ElementType::object_rectangle;
    if (equals(str, "DOMSymbolInstance")) return r = ElementType::symbol_instance;
    if (equals(str, "DOMDynamicText")) return r = ElementType::dynamic_text;
    if (equals(str, "DOMStaticText")) return r = ElementType::static_text;
    if (equals(str, "DOMSymbolItem")) return r = ElementType::symbol_item;
    if (equals(str, "DOMFontItem")) return r = ElementType::font_item;
    if (equals(str, "DOMSoundItem")) return r = ElementType::sound_item;
    return r = ElementType::unknown;
}

SymbolType& operator<<(SymbolType& r, const char* str) {
    if (equals(str, "button")) return r = SymbolType::button;
    if (equals(str, "graphic")) return r = SymbolType::graphic;
    return r = SymbolType::normal;
}

LoopMode& operator<<(LoopMode& r, const char* str) {
    if (equals(str, "loop")) return r = LoopMode::loop;
    if (equals(str, "play once")) return r = LoopMode::play_once;
    if (equals(str, "single frame")) return r = LoopMode::single_frame;
    return r = LoopMode::none;
}

/** [parsing classes] **/

ItemProperties& operator<<(ItemProperties& r, const xml_node& node) {
    r.name = node.attribute("name").value();
    r.itemID = node.attribute("itemID").value();
    r.sourceLastImported = node.attribute("sourceLastImported").as_int();
    r.sourceExternalFilepath = node.attribute("sourceExternalFilepath").value();
    r.linkageClassName = node.attribute("linkageClassName").value();
    r.linkageExportForAS = node.attribute("linkageExportForAS").as_bool(false);
    r.linkageBaseClass = node.attribute("linkageBaseClass").value();
    r.lastModified = node.attribute("lastModified").as_int();
    return r;
}

FolderItem& operator<<(FolderItem& r, const xml_node& node) {
    r.item << node;
    return r;
}

vec2_t read_alignment(const xml_node& node) {
    vec2_t r = {{0.0f, 0.0f}};
    const char* alignment = node.attribute("alignment").value();
    if (equals(alignment, "center")) {
        r.x = 0.5f;
    } else if (equals(alignment, "right")) {
        r.x = 1.0f;
    }
    return r;
}

TextAttributes& operator<<(TextAttributes& r, const xml_node& node) {
    r.alignment = read_alignment(node);
    r.aliasText = node.attribute("aliasText").as_bool();
    r.bold = node.attribute("bold").as_bool();
    r.italic = node.attribute("italic").as_bool();
    r.size = node.attribute("size").as_float(12.0f);
    r.lineHeight = node.attribute("lineHeight").as_float(r.size);
    r.lineSpacing = node.attribute("lineSpacing").as_float(0.0f);
    r.bitmapSize = node.attribute("bitmapSize").as_uint((uint32_t)(r.size * 20));
    r.autoKern = node.attribute("autoKern").as_bool(true);

    r.face = node.attribute("face").value();
    r.color = read_color(node, "fillColor", "alpha");

    return r;
}

TextRun& operator<<(TextRun& r, const xml_node& node) {
    r.characters = node.child("characters").text().as_string();
    r.attributes << node.child("textAttrs").first_child();
    return r;
}

DocInfo& operator<<(DocInfo& r, const xml_node& node) {
    r.width = node.attribute("width").as_int(550);
    r.height = node.attribute("height").as_int(400);
    r.backgroundColor = parse_css_color(node.attribute("backgroundColor").as_string());
    r.frameRate = node.attribute("frameRate").as_int(24);
    r.currentTimeline = node.attribute("currentTimeline").as_int();
    r.xflVersion = node.attribute("xflVersion").as_float();
    r.majorVersion = node.attribute("majorVersion").as_int();
    r.buildNumber = node.attribute("buildNumber").as_int();
    r.viewAngle3D = node.attribute("viewAngle3D").as_float();
    return r;
}


/** Graphics objects parsing **/

ScaleMode& operator<<(ScaleMode& r, const char* str) {
    if (equals(str, "normal")) return r = ScaleMode::normal;
    else if (equals(str, "horizontal")) return r = ScaleMode::horizontal;
    else if (equals(str, "vertical")) return r = ScaleMode::vertical;
    else if (equals(str, "none")) return r = ScaleMode::none;
    else if (str && *str) EK_WARN("unknown ScaleMode: %s", str);
    return r = ScaleMode::none;
}

SolidStyleType& operator<<(SolidStyleType& r, const char* str) {
    if (equals(str, "hairline")) return r = SolidStyleType::hairline;
    else if (str && *str) EK_WARN("unknown SolidStyle: %s", str);
    return r = SolidStyleType::hairline;
}

LineCaps& operator<<(LineCaps& r, const char* str) {
    if (equals(str, "none")) return r = LineCaps::none;
    else if (equals(str, "round")) return r = LineCaps::round;
    else if (equals(str, "square")) return r = LineCaps::square;
    else if (str && *str) EK_WARN("unknown LineCaps: %s", str);
    return r = LineCaps::round;
}

LineJoints& operator<<(LineJoints& r, const char* str) {
    if (equals(str, "miter")) return r = LineJoints::miter;
    else if (equals(str, "round")) return r = LineJoints::round;
    else if (equals(str, "bevel")) return r = LineJoints::bevel;
    else if (str && *str) EK_WARN("unknown LineJoints: %s", str);
    return r = LineJoints::round;
}

FillType& operator<<(FillType& r, const char* str) {
    if (equals(str, "SolidColor")) return r = FillType::solid;
    if (equals(str, "LinearGradient")) return r = FillType::linear;
    if (equals(str, "RadialGradient")) return r = FillType::radial;
    if (equals(str, "BitmapFill")) return r = FillType::bitmap;
    return r = FillType::unknown;
}

SpreadMethod& operator<<(SpreadMethod& r, const char* str) {
    //const char* str = node.attribute("spreadMethod").value();
    if (equals(str, "reflect")) return r = SpreadMethod::reflect;
    if (equals(str, "repeat")) return r = SpreadMethod::repeat;
    return r = SpreadMethod::extend;
}

GradientEntry& operator<<(GradientEntry& r, const xml_node& node) {
    r.color = read_color(node);
    r.ratio = node.attribute("ratio").as_float();
    return r;
}

/** Blend Mode parsing **/


BlendMode& operator<<(BlendMode& r, const char* str) {
    if (!str || !(*str)) return r = BlendMode::last;
    else if (equals(str, "normal")) return r = BlendMode::normal;
    else if (equals(str, "layer")) return r = BlendMode::layer;
    else if (equals(str, "multiply")) return r = BlendMode::multiply;
    else if (equals(str, "screen")) return r = BlendMode::screen;
    else if (equals(str, "lighten")) return r = BlendMode::lighten;
    else if (equals(str, "darken")) return r = BlendMode::darken;
    else if (equals(str, "difference")) return r = BlendMode::difference;
    else if (equals(str, "add")) return r = BlendMode::add;
    else if (equals(str, "subtract")) return r = BlendMode::subtract;
    else if (equals(str, "invert")) return r = BlendMode::invert;
    else if (equals(str, "alpha")) return r = BlendMode::alpha;
    else if (equals(str, "erase")) return r = BlendMode::erase;
    else if (equals(str, "overlay")) return r = BlendMode::overlay;
    else if (equals(str, "hardlight")) return r = BlendMode::hardlight;
    EK_WARN("unknown BlendMode: %s", str);
    return r = BlendMode::normal;
}

/** Filters parsing **/

FilterType& operator<<(FilterType& r, const char* str) {
    if (equals(str, "DropShadowFilter")) {
        return r = FilterType::drop_shadow;
    } else if (equals(str, "GlowFilter")) {
        return r = FilterType::glow;
    } else if (equals(str, "BevelFilter")) {
        return r = FilterType::bevel;
    } else if (equals(str, "BlurFilter")) {
        return r = FilterType::blur;
    } else if (equals(str, "ConvolutionFilter")) {
        return r = FilterType::convolution;
    } else if (equals(str, "AdjustColorFilter")) {
        return r = FilterType::adjust_color;
    } else if (equals(str, "GradientBevelFilter")) {
        return r = FilterType::bevel;
    } else if (equals(str, "GradientGlowFilter")) {
        return r = FilterType::glow;
    }
    return r = FilterType::none;
}

Filter& operator<<(Filter& r, const xml_node& node) {
    r.type << node.name();
    r.color = read_color(node);
    r.blur.x = node.attribute("blurX").as_float(4.0f);
    r.blur.y = node.attribute("blurY").as_float(4.0f);
    r.distance = node.attribute("distance").as_float(4.0f);
    r.angle = node.attribute("angle").as_float(45.0f);
    r.quality = node.attribute("quality").as_int(1);
    r.strength = node.attribute("strength").as_float(1.0f);
    r.inner = node.attribute("inner").as_bool(false);
    r.knockout = node.attribute("knockout").as_bool(false);
    r.hideObject = node.attribute("hideObject").as_bool(false);
    return r;
}

/** Shape Edges parsing **/


Array<std::string> split(const std::string& str, const char separator) {
    using size_type = std::string::size_type;

    Array<std::string> result{};
    size_type prevPos = 0;
    size_type pos = 0;

    while ((pos = str.find(separator, pos)) != std::string::npos) {
        std::string substring{str.substr(prevPos, pos - prevPos)};
        result.push_back(substring);
        prevPos = ++pos;
    }

    // Last word
    result.push_back(str.substr(prevPos, pos - prevPos));

    return result;
}

double read_double_hex(const char* str, int len) {
    if (len == 0) {
        return 0.0;
    }

    if (str[0] == '#') {
        char* dot = nullptr;
        uint32_t hex = strtoul(str + 1, &dot, 16);
        hex = hex << 8;
        if (dot && dot[0] == '.') {
            auto n = len - int(dot - str + 1);
            uint32_t postfix = strtoul(dot + 1, nullptr, 16);
            while (n < 2) {
                // each trailing zero is 4-bit
                postfix = postfix << 4;
                ++n;
            }
            hex = hex | postfix;
        }
        return (*reinterpret_cast<int32_t*>(&hex)) / double(1 << 8);
    }

    // default floating point format
    return strtod(str, nullptr);
}

double read_twips(const char* str, int n) {
    return read_double_hex(str, n) / 20.0;
}

bool is_whitespace(char c) {
    return c == '\n' || c == '\r' || c == ' ' || c == '\t';
}

bool is_cmd(char c) {
    return c == '!' || c == '|' || c == '/' || c == '[' || c == ']' || c == 'S';
}

bool is_eos(char c) {
    return c == '\0';
}

int get_value_length(const char* buf) {
    const char* ptr = buf;
    char c = *ptr;
    while (!is_eos(c) && !is_cmd(c) && !is_whitespace(c)) {
        ++ptr;
        c = *ptr;
    }
    return ptr - buf;
}

void parse_edges(const char* data, Array<char>& out_commands, Array<double>& out_values) {
    if (!data) {
        return;
    }

    char c = *data;
    while (!is_eos(c)) {
        if (is_whitespace(c)) {

        } else if (is_cmd(c)) {
            out_commands.push_back(c);
        } else {
            int l = get_value_length(data);
            out_values.push_back(read_twips(data, l));
            data += l - 1;
        }

        ++data;
        c = *data;
    }
}

Edge& operator<<(Edge& r, const xml_node& node) {
    parse_edges(node.attribute("edges").value(), r.commands, r.values);
    r.fill_style_0 = node.attribute("fillStyle0").as_int();
    r.fill_style_1 = node.attribute("fillStyle1").as_int();
    r.stroke_style = node.attribute("strokeStyle").as_int();
    return r;
}

}