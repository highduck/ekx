#include "elements_parse.h"

#include "filters.h"
#include "edges.h"
#include "graphics_parse.h"
#include "basic_types.h"
#include "parse_blend_mode.h"

#include <ek/flash/doc/element_types.h>
#include <ek/flash/doc/filter.h>
#include <ek/flash/doc/graphic_types.h>

#include <pugixml.hpp>

namespace ek::flash {

layer_type& operator<<(layer_type& r, const char* str) {
    if (equals(str, "guide")) return r = layer_type::guide;
    return r = layer_type::normal;
}

tween_type& operator<<(tween_type& r, const char* str) {
    if (equals(str, "motion")) return r = tween_type::motion;
    // TODO:
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

element_t& operator<<(element_t& r, const xml_node& node) {
    r.item << node;
    r.elementType << node.name();

    r.rect = read_rect(node);

    //// shape
    r.isDrawingObject = node.attribute("isDrawingObject").as_bool();
    for (const auto& el: node.child("fills").children("FillStyle")) {
        r.fills.push_back(parse_xml_node<fill_style>(el));
    }

    for (const auto& el: node.child("strokes").children("StrokeStyle")) {
        r.strokes.push_back(parse_xml_node<stroke_style>(el));
    }

    for (const auto& el: node.child("edges").children("Edge")) {
        r.edges.push_back(parse_xml_node<edge_t>(el));
    }

    /// instances ref
    r.libraryItemName = node.attribute("libraryItemName").value();

    /////   SymbolInstance
    r.symbolType << node.attribute("symbolType").value();
    r.centerPoint3DX = node.attribute("centerPoint3DX").as_float();
    r.centerPoint3DY = node.attribute("centerPoint3DY").as_float();
    r.cacheAsBitmap = node.attribute("cacheAsBitmap").as_bool();
    r.exportAsBitmap = node.attribute("exportAsBitmap").as_bool();
    bool hasAccessibleData = node.attribute("hasAccessibleData").as_bool();
    if (hasAccessibleData) {
        r.forceSimple = node.attribute("forceSimple").as_bool();
        r.silent = node.attribute("silent").as_bool();
    }
    r.isVisible = node.attribute("isVisible").as_bool(true);

    /// text
    r.isSelectable = node.attribute("isSelectable").as_bool(true);

    /// dynamic text
    r.border = node.attribute("border").as_bool();
    r.fontRenderingMode = node.attribute("fontRenderingMode").value();
    r.autoExpand = node.attribute("autoExpand").as_bool();
    r.lineType = node.attribute("lineType").value();
    r.transformationPoint = read_transformation_point(node);
    r.color << node;
    r.matrix << node;

    //// group
    for (const auto& member: node.child("members").children()) {
        r.members.emplace_back(parse_xml_node<element_t>(member));
    }

    for (const auto& tr: node.child("textRuns").children("DOMTextRun")) {
        r.textRuns.push_back(parse_xml_node<text_run_t>(tr));
    }

    for (const auto& tr: node.child("filters").children()) {
        r.filters.push_back(parse_xml_node<filter_t>(tr));
    }

    //// symbol item
    r.scaleGrid = read_scale_grid(node);
    r.timeline = parse_xml_node<timeline_t>(node.child("timeline").child("DOMTimeline"));
    r.blend_mode << node.attribute("blendMode").value();

    // bitmap item
    r.quality = node.attribute("quality").as_int(100);
    r.href = node.attribute("href").value();
    r.bitmapDataHRef = node.attribute("bitmapDataHRef").value();
    r.isJPEG = node.attribute("isJPEG").as_bool(false);

    // font item
    r.font = node.attribute("font").value();
    r.size = node.attribute("size").as_int();
    r.id = node.attribute("id").as_int();

    // sound item
    ////  todo:


    return r;
}

frame_t& operator<<(frame_t& r, const xml_node& node) {
    r.index = node.attribute("index").as_int();
    r.duration = node.attribute("duration").as_int(1);
    r.tweenType << node.attribute("tweenType").value();
    r.name = node.attribute("name").value();

    r.motionTweenSnap = node.attribute("motionTweenSnap").as_bool(false);
    r.motionTweenOrientToPath = node.attribute("motionTweenOrientToPath").as_bool(false);

    r.motionTweenRotate << node.attribute("motionTweenRotate").value();
    r.motionTweenRotateTimes = node.attribute("motionTweenRotateTimes").as_int(0);

    r.hasCustomEase = node.attribute("hasCustomEase").as_bool(false);

    r.keyMode = node.attribute("keyMode").as_int(0);
    r.acceleration = node.attribute("acceleration").as_int(0);

    for (const auto& item : node.child("elements").children()) {
        r.elements.push_back(parse_xml_node<element_t>(item));
    }

    for (const auto& item : node.child("tweens").children()) {
        tween_target target;
        target << item.attribute("target").as_string();
        tween_object_t* tween_ptr = nullptr;
        for (auto& t : r.tweens) {
            if (t.target == target) {
                tween_ptr = &t;
                break;
            }
        }
        if (!tween_ptr) {
            tween_ptr = &r.tweens.emplace_back();
        }

        if (strcmp(item.name(), "CustomEase") == 0) {
            for (const auto& point_node : item.children("Point")) {
                tween_ptr->custom_ease.push_back(read_point(point_node));
            }
        } else if (strcmp(item.name(), "Ease") == 0) {
            tween_ptr->custom_ease.clear();
            tween_ptr->intensity = item.attribute("intensity").as_int(0);
        }
    }
    return r;
}

layer_t& operator<<(layer_t& r, const xml_node& node) {
    r.name = node.attribute("name").value();
    r.layerType << node.attribute("layerType").value();
    for (const auto& item: node.child("frames").children("DOMFrame")) {
        r.frames.push_back(parse_xml_node<frame_t>(item));
    }
    return r;
}

timeline_t& operator<<(timeline_t& r, const xml_node& node) {
    r.name = node.attribute("name").value();
    for (const auto& item : node.child("layers").children("DOMLayer")) {
        r.layers.push_back(parse_xml_node<layer_t>(item));
    }
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