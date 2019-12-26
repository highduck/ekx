#include "serialize.hpp"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace ek {

std::string to_json_str(const font_data_t& font) {
    json json_glyphs = json::array();
    for (auto& g : font.glyphs) {
        json_glyphs.emplace_back(json{
                {"codes",         g.codes},
                {"box",           g.box},
                {"advance_width", g.advance_width},
                {"sprite",        g.sprite.empty() ? nullptr : g.sprite}
        });
    }
    json data = {
            {"units_per_em", font.units_per_em},
            {"glyphs",       json_glyphs},
            {"sizes",        font.sizes}
    };
    return data.dump(4);
}

json to_json(const matrix_2d& m) {
    return json::array({m.a, m.b, m.c, m.d, m.tx, m.ty});
}

json to_json(const float4& v4) {
    return json::array({
                               v4.r,
                               v4.g,
                               v4.b,
                               v4.a
                       });
}

json to_json(const rect_f& rc) {
    return json::array({rc.x, rc.y, rc.width, rc.height});
}

json to_json(const float2& v) {
    return json::array({v.x, v.y});
}

json to_json_opt(const std::string& str) {
    json r = nullptr;
    if (!str.empty()) {
        r = str;
    }
    return r;
}

json to_json(const sg_node_data& node);

json to_json(const std::vector<sg_node_data>& nodes) {
    json arr = json::array();
    for (const auto& n : nodes) {
        arr.emplace_back(to_json(n));
    }
    return arr;
}

json to_json(const dynamic_text_data& data) {
    json r = {
            {"rect",         to_json(data.rect)},
            {"text",         data.text},
            {"face",         data.face},
            {"alignment",    to_json(data.alignment)},
            {"line_spacing", data.line_spacing},
            {"line_height",  data.line_height},
            {"size",         data.size},
            {"color",        data.color.argb}
    };
    return r;
}

json to_json(const std::vector<float2>& points) {
    json r = json::array();
    for (const auto& point : points) {
        r.emplace_back(point.x);
        r.emplace_back(point.y);
    }
    return r;
}

json to_json(const std::vector<filter_data>& filters) {
    json r = json::array();
    for (const auto& filter : filters) {
        json e = {
                {"type",    filter.type},
                {"quality", filter.quality},
                {"color",   filter.color.argb},
                {"blur",    to_json(filter.blur)},
                {"offset",  to_json(filter.offset)}
        };
        r.emplace_back(e);
    }
    return r;
}

json to_json(const std::vector<easing_data_t>& easings) {
    json r = json::array();
    for (const auto& easing : easings) {
        json e = json::object();
        if (easing.attribute != 0) {
            e["attribute"] = easing.attribute;
        }
        if (easing.ease != 0.0f) {
            e["ease"] = easing.ease;
        }
        if (!easing.curve.empty()) {
            e["curve"] = to_json(easing.curve);
        }
        r.emplace_back(e);
    }
    return r;
}

json to_json(const std::vector<movie_frame_data>& frames) {
    json r = json::array();
    for (const auto& frame : frames) {
        json e = {
                {"index",       frame.index},
                {"duration",    frame.duration},
                {"motion_type", frame.motion_type},
                {"key",         frame.key}
        };
        if (frame.position != float2::zero) {
            e["pos"] = to_json(frame.position);
        }
        if (frame.scale != float2::one) {
            e["scale"] = to_json(frame.scale);
        }
        if (frame.skew != float2::zero) {
            e["skew"] = to_json(frame.skew);
        }
        if (frame.pivot != float2::zero) {
            e["pivot"] = to_json(frame.pivot);
        }
        if (frame.color.multiplier != float4::one) {
            e["color_mul"] = to_json(frame.color.multiplier);
        }
        if (frame.color.offset != float4::zero) {
            e["color_off"] = to_json(frame.color.offset);
        }
        if (!frame.tweens.empty()) {
            e["tweens"] = to_json(frame.tweens);
        }
        r.emplace_back(e);
    }
    return r;
}

json to_json(const std::vector<movie_layer_data>& layers) {
    json r = json::array();
    for (const auto& layer : layers) {
        json e = {
                {"key",    layer.key},
                {"frames", to_json(layer.frames)}
        };
        r.emplace_back(e);
    }
    return r;
}

json to_json(const sg_movie_data& data) {
    json r = {
            {"frames", data.frames},
            {"layers", to_json(data.layers)}
    };
    return r;
}

json to_json(const sg_node_data& node) {
    const auto pos = node.matrix.position();
    const auto scale = node.matrix.scale();
    const auto skew = node.matrix.skew();

    json r = json::object();

    if (pos != float2::zero) {
        r["pos"] = to_json(pos);
    }
    if (scale != float2::one) {
        r["scale"] = to_json(scale);
    }
    if (skew != float2::zero) {
        r["skew"] = to_json(skew);
    }
    if (!node.name.empty()) {
        r["name"] = node.name;
    }
    if (!node.libraryName.empty()) {
        r["lib_name"] = node.libraryName;
    }
    if (node.color.multiplier != float4::one) {
        r["color_mul"] = to_json(node.color.multiplier);
    }
    if (node.color.offset != float4::zero) {
        r["color_off"] = to_json(node.color.offset);
    }
    // matrix
    if (node.button) {
        r["button"] = node.button;
    }
    if (!node.touchable) {
        r["touchable"] = node.touchable;
    }
    if (!node.visible) {
        r["visible"] = node.visible;
    }
    if (node.animationKey != 0) {
        r["animationKey"] = node.animationKey;
    }
    if (node.layerKey != 0) {
        r["layerKey"] = node.layerKey;
    }
    if (!node.sprite.empty()) {
        r["sprite"] = node.sprite;
    }
    if (!node.scaleGrid.empty()) {
        r["scaleGrid"] = to_json(node.scaleGrid);
    }
    if (!node.hitRect.empty()) {
        r["hitRect"] = to_json(node.hitRect);
    }
    if (!node.clipRect.empty()) {
        r["clipRect"] = to_json(node.clipRect);
    }
    if (node.dynamicText.has_value()) {
        r["dynamicText"] = to_json(*node.dynamicText);
    }
    if (node.movie.has_value()) {
        r["movie"] = to_json(*node.movie);
    }
    if (!node.script.empty()) {
        r["script"] = node.script;
    }
    if (!node.filters.empty()) {
        r["filters"] = to_json(node.filters);
    }
    if (!node.children.empty()) {
        r["children"] = to_json(node.children);
    }
    return r;
}

std::string to_json_str(const sg_file& ani) {
    json data = {
            {"linkages", ani.linkages},
            {"library",  to_json(ani.library)}
    };
    return data.dump(4);
}

}