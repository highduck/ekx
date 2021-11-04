#include "Widgets.hpp"
#include "../imgui/imgui.hpp"

namespace ImGui {

void HelpMarker(const char* desc)
{
    ImGui::TextDisabled(ICON_FA_QUESTION_CIRCLE);
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

bool Color32Edit(const char* label, ek::argb32_t& argb) {
    ek::float4 v{argb};
    bool changed = ImGui::ColorEdit4(label, v.data());
    if (changed) {
        argb = ek::argb32_t{v};
    }
    return changed;
}

bool EditRect(const char* label, float * xywh, float v_speed, const char* format, float power) {
    ImGui::PushID(xywh);
    ImGui::LabelText(label, "x: %0.2f y: %0.2f w: %0.2f h: %0.2f", xywh[0], xywh[1], xywh[2], xywh[3]);
    bool changed = ImGui::DragFloat2("Position", xywh, v_speed, 0, 0, format, power);
    changed = changed || ImGui::DragFloat2("Size", xywh + 2, v_speed, 0, FLT_MAX, format, power);
    ImGui::PopID();
    return changed;
}

bool ToolbarButton(const char* label, bool active, const char* tooltip) {
    PushStyleColor(ImGuiCol_Button, active ? 0xFFFF7700 : 0x11111111);
    PushStyleColor(ImGuiCol_Text, active ? 0xFFFFFFFF : 0xFFCCCCCC);
    bool res = Button(label);
    if(tooltip && IsItemHovered()) {
        SetTooltip("%s", tooltip);
    }
    PopStyleColor(2);
    return res;
}

}

namespace ek {

std::string getDebugNodePath(ecs::EntityApi e) {
    std::string result{};
    while (e) {
        result = e.get_or_default<NodeName>().name + "/" + result;
        e = e.get<Node>().parent;
    }
    return result;
}

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


void guiSprite(const Sprite& sprite) {
    auto rc = sprite.rect;
    auto uv0 = sprite.tex.position;
    auto uv1 = sprite.tex.right_bottom();
    if (sprite.texture) {
        void* tex_id = (void*)(uintptr_t)sprite.texture->image.id;
        if (sprite.rotated) {
            ImGui::BeginChild("s", ImVec2{rc.width, rc.height});
            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            auto pos = ImGui::GetCursorScreenPos();
            draw_list->AddImageQuad(tex_id,
                                    pos,
                                    ImVec2{pos.x + rc.width, pos.y},
                                    ImVec2{pos.x + rc.width, pos.y + rc.height},
                                    ImVec2{pos.x, pos.y + rc.height},
                                    ImVec2{uv0.x, uv1.y},
                                    ImVec2{uv0.x, uv0.y},
                                    ImVec2{uv1.x, uv0.y},
                                    ImVec2{uv1.x, uv1.y},
                                    IM_COL32_WHITE);
            ImGui::EndChild();
        } else {
            ImGui::Image(
                    tex_id,
                    ImVec2{rc.width, rc.height},
                    ImVec2{uv0.x, uv0.y},
                    ImVec2{uv1.x, uv1.y}
            );
        }
    } else {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "Sprite Texture is NULL");
    }
}

void guiFont(const Font& font) {
    switch (font.getFontType()) {
        case FontType::Bitmap: {
            auto* bm = reinterpret_cast<const BitmapFont*>(font.getImpl());
            ImGui::Text("Font Type: Bitmap");
            ImGui::Text("Glyphs: %lu", bm->map.size());
        }
            break;
        case FontType::TrueType: {
            auto* ttf = reinterpret_cast<const TrueTypeFont*>(font.getImpl());
            ImGui::Text("Font Type: TrueType");
            if(ttf->map) {
                ImGui::Text("Glyphs: %lu", ttf->map->size());
            }
        }
            break;
    }
}

}