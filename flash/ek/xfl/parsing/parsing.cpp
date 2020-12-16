#include "parsing.hpp"

#include <ek/xfl/types.hpp>
#include <ek/util/logger.hpp>

#include <pugixml.hpp>

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

rect_f read_rect(const xml_node& node) {
    return {
            node.attribute("left").as_float(),
            node.attribute("top").as_float(),
            node.attribute("width").as_float(),
            node.attribute("height").as_float()
    };
}

static rect_f read_rect_bounds(const xml_node& node, const std::array<const char*, 4>& nn) {
    return min_max_box<float, 2>(
            {node.attribute(nn[0]).as_float(), node.attribute(nn[1]).as_float()},
            {node.attribute(nn[2]).as_float(), node.attribute(nn[3]).as_float()}
    );
}

rect_f read_scale_grid(const xml_node& node) {
    return read_rect_bounds(node, {
            "scaleGridLeft",
            "scaleGridTop",
            "scaleGridRight",
            "scaleGridBottom"
    });
}

float2 read_point(const xml_node& node) {
    return {node.attribute("x").as_float(), node.attribute("y").as_float()};
}

float2 read_transformation_point(const xml_node& node) {
    return read_point(node.child("transformationPoint").child("Point"));
}

matrix_2d& operator<<(matrix_2d& r, const xml_node& node) {
    const auto& m = node.child("matrix").child("Matrix");
    r.a = m.attribute("a").as_float(1.0f);
    r.b = m.attribute("b").as_float();
    r.c = m.attribute("c").as_float();
    r.d = m.attribute("d").as_float(1.0f);
    r.tx = m.attribute("tx").as_float();
    r.ty = m.attribute("ty").as_float();
    return r;
}

color_transform_f& operator<<(color_transform_f& color, const xml_node& node) {
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
    const auto br = math::clamp(ct.attribute("brightness").as_float(0.0f), -1.0f, 1.0f);
    if (br < 0.0f) {
        color.scale.x =
        color.scale.y =
        color.scale.z = 1.0f + br;
    } else if (br > 0.0f) {
        color.offset.x =
        color.offset.y =
        color.offset.z = br;
    }

    return color;
}

static float4 read_color(const xml_node& node, const char* color_tag = "color", const char* alpha_tag = "alpha") {
    const auto c = parse_css_color(node.attribute(color_tag).value());
    return {
            static_cast<float>((c >> 16u) & 0xFFu) / 255.0f,
            static_cast<float>((c >> 8u) & 0xFFu) / 255.0f,
            static_cast<float>(c & 0xFFu) / 255.0f,
            node.attribute(alpha_tag).as_float(1.0f)
    };
}

/** Element parsing **/
layer_type& operator<<(layer_type& r, const char* str) {
    if (equals(str, "guide")) return r = layer_type::guide;
    return r = layer_type::normal;
}

tween_type& operator<<(tween_type& r, const char* str) {
    if (equals(str, "motion")) return r = tween_type::classic;
    if (equals(str, "motion object")) return r = tween_type::motion_object;
    return r = tween_type::none;
}

tween_target& operator<<(tween_target& r, const char* str) {
    if (equals(str, "all")) return r = tween_target::all;
    if (equals(str, "position")) return r = tween_target::position;
    if (equals(str, "rotation")) return r = tween_target::rotation;
    if (equals(str, "scale")) return r = tween_target::scale;
    if (equals(str, "color")) return r = tween_target::color;
    if (equals(str, "filters")) return r = tween_target::filters;
    return r = tween_target::all;
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

rotation_direction& operator<<(rotation_direction& r, const char* str) {
    if (equals(str, "counter-clockwise")) return r = rotation_direction::ccw;
    else if (equals(str, "clockwise")) return r = rotation_direction::cw;
    return r = rotation_direction::none;
}

element_type& operator<<(element_type& r, const char* str) {
    if (equals(str, "DOMGroup")) return r = element_type::group;
    if (equals(str, "DOMBitmapInstance")) return r = element_type::bitmap_instance;
    if (equals(str, "DOMBitmapItem")) return r = element_type::bitmap_item;
    if (equals(str, "DOMShape")) return r = element_type::shape;
    if (equals(str, "DOMOvalObject")) return r = element_type::object_oval;
    if (equals(str, "DOMRectangleObject")) return r = element_type::object_rectangle;
    if (equals(str, "DOMSymbolInstance")) return r = element_type::symbol_instance;
    if (equals(str, "DOMDynamicText")) return r = element_type::dynamic_text;
    if (equals(str, "DOMStaticText")) return r = element_type::static_text;
    if (equals(str, "DOMSymbolItem")) return r = element_type::symbol_item;
    if (equals(str, "DOMFontItem")) return r = element_type::font_item;
    if (equals(str, "DOMSoundItem")) return r = element_type::sound_item;
    return r = element_type::unknown;
}

symbol_type& operator<<(symbol_type& r, const char* str) {
    if (equals(str, "button")) return r = symbol_type::button;
    if (equals(str, "graphic")) return r = symbol_type::graphic;
    return r = symbol_type::normal;
}

loop_mode& operator<<(loop_mode& r, const char* str) {
    if (equals(str, "loop")) return r = loop_mode::loop;
    if (equals(str, "play once")) return r = loop_mode::play_once;
    if (equals(str, "single frame")) return r = loop_mode::single_frame;
    return r = loop_mode::none;
}

/** [parsing classes] **/

item_properties& operator<<(item_properties& r, const xml_node& node) {
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

folder_item& operator<<(folder_item& r, const xml_node& node) {
    r.item << node;
    return r;
}

float2 read_alignment(const xml_node& node) {
    float2 r{0.0f, 0.0f};
    const char* alignment = node.attribute("alignment").value();
    if (equals(alignment, "center")) {
        r.x = 0.5f;
    } else if (equals(alignment, "right")) {
        r.x = 1.0f;
    }
    return r;
}

text_attributes_t& operator<<(text_attributes_t& r, const xml_node& node) {
    r.alignment = read_alignment(node);
    r.aliasText = node.attribute("aliasText").as_bool();
    r.bold = node.attribute("bold").as_bool();
    r.italic = node.attribute("italic").as_bool();
    r.size = node.attribute("size").as_float(12.0f);
    r.lineHeight = node.attribute("lineHeight").as_float(r.size);
    r.lineSpacing = node.attribute("lineSpacing").as_float(0.0f);
    r.bitmapSize = node.attribute("bitmapSize").as_int(static_cast<uint32_t>(r.size * 20u));
    r.autoKern = node.attribute("autoKern").as_bool(true);

    r.face = node.attribute("face").value();
    r.color = read_color(node, "fillColor", "alpha");

    return r;
}

text_run_t& operator<<(text_run_t& r, const xml_node& node) {
    r.characters = node.child("characters").text().as_string();
    r.attributes << node.child("textAttrs").first_child();
    return r;
}

document_info& operator<<(document_info& r, const xml_node& node) {
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

scale_mode& operator<<(scale_mode& r, const char* str) {
    if (equals(str, "normal")) return r = scale_mode::normal;
    else if (equals(str, "horizontal")) return r = scale_mode::horizontal;
    else if (equals(str, "vertical")) return r = scale_mode::vertical;
    else if (equals(str, "none")) return r = scale_mode::none;
    else if (str && *str) EK_WARN("unknown ScaleMode: %s", str);
    return r = scale_mode::none;
}

solid_style_type& operator<<(solid_style_type& r, const char* str) {
    if (equals(str, "hairline")) return r = solid_style_type::hairline;
    else if (str && *str) EK_WARN("unknown SolidStyle: %s", str);
    return r = solid_style_type::hairline;
}

line_caps& operator<<(line_caps& r, const char* str) {
    if (equals(str, "none")) return r = line_caps::none;
    else if (equals(str, "round")) return r = line_caps::round;
    else if (equals(str, "square")) return r = line_caps::square;
    else if (str && *str) EK_WARN("unknown LineCaps: %s", str);
    return r = line_caps::round;
}

line_joints& operator<<(line_joints& r, const char* str) {
    if (equals(str, "miter")) return r = line_joints::miter;
    else if (equals(str, "round")) return r = line_joints::round;
    else if (equals(str, "bevel")) return r = line_joints::bevel;
    else if (str && *str) EK_WARN("unknown LineJoints: %s", str);
    return r = line_joints::round;
}

fill_type& operator<<(fill_type& r, const char* str) {
    if (equals(str, "SolidColor")) return r = fill_type::solid;
    if (equals(str, "LinearGradient")) return r = fill_type::linear;
    if (equals(str, "RadialGradient")) return r = fill_type::radial;
    if (equals(str, "BitmapFill")) return r = fill_type::bitmap;
    return r = fill_type::unknown;
}

spread_method& operator<<(spread_method& r, const char* str) {
    //const char* str = node.attribute("spreadMethod").value();
    if (equals(str, "reflect")) return r = spread_method::reflect;
    if (equals(str, "repeat")) return r = spread_method::repeat;
    return r = spread_method::extend;
}

gradient_entry& operator<<(gradient_entry& r, const xml_node& node) {
    r.color = read_color(node);
    r.ratio = node.attribute("ratio").as_float();
    return r;
}

/** Blend Mode parsing **/


blend_mode_t& operator<<(blend_mode_t& r, const char* str) {
    if (!str || !(*str)) return r = blend_mode_t::last;
    else if (equals(str, "normal")) return r = blend_mode_t::normal;
    else if (equals(str, "layer")) return r = blend_mode_t::layer;
    else if (equals(str, "multiply")) return r = blend_mode_t::multiply;
    else if (equals(str, "screen")) return r = blend_mode_t::screen;
    else if (equals(str, "lighten")) return r = blend_mode_t::lighten;
    else if (equals(str, "darken")) return r = blend_mode_t::darken;
    else if (equals(str, "difference")) return r = blend_mode_t::difference;
    else if (equals(str, "add")) return r = blend_mode_t::add;
    else if (equals(str, "subtract")) return r = blend_mode_t::subtract;
    else if (equals(str, "invert")) return r = blend_mode_t::invert;
    else if (equals(str, "alpha")) return r = blend_mode_t::alpha;
    else if (equals(str, "erase")) return r = blend_mode_t::erase;
    else if (equals(str, "overlay")) return r = blend_mode_t::overlay;
    else if (equals(str, "hardlight")) return r = blend_mode_t::hardlight;
    EK_WARN("unknown BlendMode: %s", str);
    return r = blend_mode_t::normal;
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


std::vector<std::string> split(const std::string& str, const char separator) {
    using size_type = std::string::size_type;

    std::vector<std::string> result;
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

void parse_edges(const char* data, std::vector<char>& out_commands, std::vector<double>& out_values) {
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

edge_t& operator<<(edge_t& r, const xml_node& node) {
    parse_edges(node.attribute("edges").value(), r.commands, r.values);
    r.fill_style_0 = node.attribute("fillStyle0").as_int();
    r.fill_style_1 = node.attribute("fillStyle1").as_int();
    r.stroke_style = node.attribute("strokeStyle").as_int();
    return r;
}

}