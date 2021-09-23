#pragma once

#include "types.h"
#include <cstdio>
#include <cstring>
#include <pugixml.hpp>

namespace bmfont_export {

FilterType createFilterType(const char* name) {
    if (!strcmp(name, "refill")) {
        return FilterType::ReFill;
    } else if (!strcmp(name, "glow")) {
        return FilterType::Glow;
    } else if (!strcmp(name, "shadow")) {
        return FilterType::Shadow;
    }
    return FilterType::None;
}

const char* toCString(FilterType type) {
    if (type == FilterType::ReFill) {
        return "refill";
    } else if (type == FilterType::Glow) {
        return "glow";
    } else if (type == FilterType::Shadow) {
        return "shadow";
    }
    return "bypass";
}

Color parseColor(const char* str) {
    uint32_t c = 0x0;
    sscanf(str, "#%08x", &c);
    return toColor(toRGBA(c));
}

void Filter::readFromXML(const pugi::xml_node& node) {
    type = createFilterType(node.attribute("type").as_string());
    top = node.attribute("top").as_float(0.0f);
    bottom = node.attribute("bottom").as_float(100.0f);
    distance = node.attribute("distance").as_float(0.0f);
    quality = node.attribute("quality").as_int(1);
    strength = node.attribute("strength").as_float(1.0f);
    angle = node.attribute("angle").as_float(0.0f);
    blurX = node.attribute("blurX").as_float(4.0f);
    blurY = node.attribute("blurY").as_float(4.0f);
    color0 = parseColor(node.attribute("color0").as_string("#FFFFFFFF"));
    color1 = parseColor(node.attribute("color1").as_string("#FF000000"));
}

void BuildBitmapFontSettings::readFromXML(const pugi::xml_node& node) {

    auto fontNode = node.child("font");
    if (fontNode) {
        fontSize = fontNode.attribute("fontSize").as_int(16);

        const CodepointPair defaultCodepointRange{0x0020u, 0x007Fu};
        for (auto& rangeNode: fontNode.children("codeRange")) {
            auto cr = defaultCodepointRange;
            cr.a = std::strtoul(rangeNode.attribute("from").as_string("0x0020"), nullptr, 16);
            cr.b = std::strtoul(rangeNode.attribute("to").as_string("0x007F"), nullptr, 16);
            ranges.push_back(cr);
        }

        if (ranges.empty()) {
            ranges.push_back(defaultCodepointRange);
        }

        mirrorCase = fontNode.attribute("mirrorCase").as_bool(false);
        useKerning = fontNode.attribute("useKerning").as_bool(true);
    }

    auto filtersNode = node.child("filters");
    if (filtersNode) {
        for (auto& filterNode: filtersNode.children("filter")) {
            Filter f{};
            f.readFromXML(filterNode);
            filters.push_back(f);
        }
    }

    auto atlasNode = node.child("atlas");
    if (atlasNode) {
        name = atlasNode.attribute("name").as_string("main");
        resolutions.clear();
        float scale = 1.0f;
        for (auto& resolutionNode: atlasNode.children("resolution")) {
            scale = resolutionNode.attribute("scale").as_float(scale);
            resolutions.push_back(scale);
            scale += 1.0f;
        }
    } else {
        name = "main";
        resolutions = {1.0f, 2.0f, 3.0f, 4.0f};
    }
}

}