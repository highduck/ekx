#include "doc_parser.hpp"
#include "parsing.hpp"

#include <pugixml.hpp>
#include <ek/util/logger.hpp>

namespace ek::flash {

xml_document* load_xml(const basic_entry& root, const path_t& path) {
    return root.open(path)->xml();
}

doc_parser::doc_parser(flash_doc& doc_, std::unique_ptr<basic_entry> root_) :
        doc{doc_},
        root{std::move(root_)} {

}

void doc_parser::parse(const xml_node& node, element_t& r) const {
    r.item << node;
    r.elementType << node.name();

    r.rect = read_rect(node);

    //// shape
    r.isDrawingObject = node.attribute("isDrawingObject").as_bool();
    for (const auto& el: node.child("fills").children("FillStyle")) {
        r.fills.push_back(read<fill_style>(el));
    }

    for (const auto& el: node.child("strokes").children("StrokeStyle")) {
        r.strokes.push_back(read<stroke_style>(el));
    }

    for (const auto& el: node.child("edges").children("Edge")) {
        r.edges.push_back(parse_xml_node<edge_t>(el));
    }

    /// instances ref
    r.libraryItemName = node.attribute("libraryItemName").value();

    /////   SymbolInstance
    r.symbolType << node.attribute("symbolType").value();
    r.loop << node.attribute("loop").value();
    r.firstFrame = node.attribute("firstFrame").as_int(0);
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
    /// dynamic text
    r.isSelectable = node.attribute("isSelectable").as_bool(true);
    r.border = node.attribute("border").as_bool();
    r.autoExpand = node.attribute("autoExpand").as_bool();
    r.scrollable = node.attribute("scrollable").as_bool();
    r.renderAsHTML = node.attribute("renderAsHTML").as_bool();
    r.fontRenderingMode << node.attribute("fontRenderingMode").value();
    r.lineType << node.attribute("lineType").value();
    r.transformationPoint = read_transformation_point(node);
    r.color << node;
    r.matrix << node;

    //// group
    for (const auto& member: node.child("members").children()) {
        r.members.push_back(read<element_t>(member));
    }

    for (const auto& tr: node.child("textRuns").children("DOMTextRun")) {
        r.textRuns.push_back(parse_xml_node<text_run_t>(tr));
    }

    for (const auto& tr: node.child("filters").children()) {
        r.filters.push_back(parse_xml_node<filter_t>(tr));
    }

    //// symbol item
    r.scaleGrid = read_scale_grid(node);
    r.timeline = read<timeline_t>(node.child("timeline").child("DOMTimeline"));
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

    switch (r.elementType) {
        case element_type::object_oval:
        case element_type::object_rectangle: {
            shape_object_t& shape = r.shape.emplace();
            parse(node, shape);

            const auto& fill = node.child("fill");
            if (!fill.empty()) {
                r.fills.push_back(read<fill_style>(fill));
            }

            const auto& stroke = node.child("stroke");
            if (!stroke.empty()) {
                r.strokes.push_back(read<stroke_style>(stroke));
            }
        }
            break;
        default:
            break;
    }
}

void doc_parser::load() {
    auto* xml = load_xml(*root, path_t{"DOMDocument.xml"});
    auto node = xml->child("DOMDocument");

    doc.info << node;

    for (const auto& item: node.child("folders").children("DOMFolderItem")) {
        doc.folders.push_back(parse_xml_node<folder_item>(item));
    }

    for (const auto& item: node.child("fonts").children("DOMFontItem")) {
        doc.library.push_back(read<element_t>(item));
    }

    for (const auto& item: node.child("media").children("DOMBitmapItem")) {
        element_t bi;
        parse(item, bi);
        bi.bitmap.reset(load_bitmap(*root->open(path_t{"bin"} / bi.bitmapDataHRef)));
        doc.library.push_back(std::move(bi));
    }

    for (const auto& item: node.child("media").children("DOMSoundItem")) {
        doc.library.push_back(read<element_t>(item));
    }

    for (const auto& item: node.child("symbols").children("Include")) {
        auto library_doc = load_xml(*root, path_t{"LIBRARY"} / item.attribute("href").value());
        auto symbol = read<element_t>(library_doc->child("DOMSymbolItem"));
        doc.library.push_back(std::move(symbol));
    }

    for (const auto& item: node.child("timelines").children("DOMTimeline")) {
        element_t el;
        el.timeline = read<timeline_t>(item);
        if (!el.timeline.name.empty()) {
            el.elementType = element_type::scene_timeline;
            el.item.name = "_SCENE_" + el.timeline.name;
            el.item.linkageExportForAS = true;
            el.item.linkageClassName = el.item.name;
            doc.scenes[el.timeline.name] = el.item.name;
            doc.library.push_back(std::move(el));
        }
    }
}

void doc_parser::parse(const xml_node& node, shape_object_t& r) {
    r.objectWidth = node.attribute("objectWidth").as_float();
    r.objectHeight = node.attribute("objectHeight").as_float();
    r.x = node.attribute("x").as_float();
    r.y = node.attribute("y").as_float();
//    fill?: DOMFillStyle;
//    stroke?: DOMStrokeStyle;

    // Oval
    r.endAngle = node.attribute("endAngle").as_float();
    r.startAngle = node.attribute("startAngle").as_float();
    r.innerRadius = node.attribute("innerRadius").as_float();
    r.closePath = node.attribute("closePath").as_bool(true);

    // Rectangle
    r.topLeftRadius = node.attribute("topLeftRadius").as_float();
    r.topRightRadius = node.attribute("topRightRadius").as_float();
    r.bottomLeftRadius = node.attribute("bottomLeftRadius").as_float();
    r.bottomRightRadius = node.attribute("bottomRightRadius").as_float();
    r.lockFlag = node.attribute("lockFlag").as_bool(false);
}

void doc_parser::parse(const xml_node& node, fill_style& r) const {
    r.index = node.attribute("index").as_int();
    for (const auto& el : node.children()) {
        r.type << el.name();
        switch (r.type) {
            case fill_type::solid:
                r.entries.push_back(parse_xml_node<gradient_entry>(el));
                break;
            case fill_type::linear:
            case fill_type::radial:
                r.spreadMethod << el.attribute("spreadMethod").value();
                r.matrix << el;
                for (const auto& e: el.children("GradientEntry")) {
                    r.entries.push_back(parse_xml_node<gradient_entry>(e));
                }
                break;
            case fill_type::bitmap:
                r.spreadMethod = spread_method::repeat;
                r.matrix << el;
                r.matrix = r.matrix.scale(1.0f / 20.0f, 1.0f / 20.0f);
                r.bitmapPath = el.attribute("bitmapPath").value();
                {
                    const auto* item = doc.find(r.bitmapPath, element_type::bitmap_item);
                    if (item && item->bitmap) {
                        r.bitmap = item->bitmap;
                    } else {
                        EK_WARN << "[BitmapFill] bitmap item not found: " << r.bitmapPath;
                    }
                }
                break;
            case fill_type::unknown:
                EK_ERROR << "Fill Style has unknown type!";
                break;
        }
        if (math::equals(det(r.matrix), 0.0f)) {
            r.type = fill_type::solid;
        }
    }
}

void doc_parser::parse(const xml_node& node, stroke_style& r) const {
    r.index = node.attribute("index").as_int();

    auto solid = node.child("SolidStroke");

    if (!solid.empty()) {
        r.is_solid = true;
        r.weight = solid.attribute("weight").as_float(1.0f);
        r.scaleMode << solid.attribute("scaleMode").value();
        parse(solid.child("fill"), r.fill);
        r.miterLimit = solid.attribute("miterLimit").as_float(3.0f);
        r.pixelHinting = solid.attribute("miterLimit").as_bool(false);
        r.caps << solid.attribute("caps").value();
        r.joints << solid.attribute("joints").value();
        r.solidStyle << solid.attribute("solidStyle").value();
    } else {
        r.is_solid = false;
    }
}

void doc_parser::parse(const xml_node& node, frame_t& r) const {
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


    r.script = node.child("Actionscript").child_value("script");

    for (const auto& item : node.child("elements").children()) {
        r.elements.push_back(read<element_t>(item));
    }

    if(!r.script.empty()) {
        if (r.script.find("valign=middle") != std::string::npos) {
            for (auto& el : r.elements) {
                if (el.elementType == element_type::dynamic_text) {
                    el.textRuns[0].attributes.alignment.y = 0.5;
                }
            }
        }
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
}

void doc_parser::parse(const xml_node& node, layer_t& r) const {
    r.name = node.attribute("name").value();
    r.layerType << node.attribute("layerType").value();
    for (const auto& item: node.child("frames").children("DOMFrame")) {
        r.frames.push_back(read<frame_t>(item));
    }
}

void doc_parser::parse(const xml_node& node, timeline_t& r) const {
    r.name = node.attribute("name").value();
    for (const auto& item : node.child("layers").children("DOMLayer")) {
        r.layers.push_back(read<layer_t>(item));
    }
}


}