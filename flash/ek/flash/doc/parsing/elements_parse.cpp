#include "parsing.hpp"

#include <ek/flash/doc/types.hpp>

#include <pugixml.hpp>

namespace ek::flash {

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
    r.alias_text = node.attribute("aliasText").as_bool();
    r.size = node.attribute("size").as_float(12.0f);
    r.line_height = node.attribute("lineHeight").as_float(r.size);
    r.line_spacing = node.attribute("lineSpacing").as_float(0.0f);
    r.bitmap_size = node.attribute("bitmapSize").as_int(static_cast<uint32_t>(r.size * 20u));
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

}