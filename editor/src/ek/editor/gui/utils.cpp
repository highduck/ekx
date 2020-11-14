#include "gui.hpp"
#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/text/text_format.hpp>

namespace ImGui {

bool Color32Edit(const char* label, ek::argb32_t& argb) {
    ek::float4 v{argb};
    bool changed = ImGui::ColorEdit4(label, v.data());
    if (changed) {
        argb = ek::argb32_t{v};
    }
    return changed;
}

bool RectEdit(const char* label, float xywh[4], float v_speed, const char* format, float power) {
    ImGui::PushID(xywh);
    ImGui::LabelText(label, "x: %f y: %f w: %f h: %f", xywh[0], xywh[1], xywh[2], xywh[3]);
    bool changed = ImGui::DragFloat2("Position", xywh, v_speed, 0, 0, format, power);
    changed = changed || ImGui::DragFloat2("Size", xywh + 2, v_speed, 0, FLT_MAX, format, power);
    ImGui::PopID();
    return changed;
}

}

namespace ek {

const char* getTextLayerTypeName(TextLayerType type) {
    static const char* names[] = {
            "Text",
            "Stroke1",
            "Stroke2",
            "Shadow"
    };
    return names[static_cast<int>(type)];
}

void guiTextLayerEffect(TextLayerEffect& layer) {
    ImGui::PushID(&layer);

    if (ImGui::CollapsingHeader(getTextLayerTypeName(layer.type))) {
        ImGui::Checkbox("Visible", &layer.visible);
        ImGui::Checkbox("Show Glyph Bounds", &layer.showGlyphBounds);
        ImGui::DragFloat("Radius", &layer.blurRadius, 1, 0, 8);
        int iterations = layer.blurIterations;
        int strength = layer.strength;
        ImGui::DragInt("Iterations", &iterations, 1, 0, 3);
        ImGui::DragInt("Strength", &strength, 1, 0, 7);
        layer.blurIterations = iterations;
        layer.strength = strength;

        ImGui::DragFloat2("Offset", layer.offset.data(), 1, 0, 8);

        ImGui::Color32Edit("Color", layer.color);
    }
    ImGui::PopID();
}

}