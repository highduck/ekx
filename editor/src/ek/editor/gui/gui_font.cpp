#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/font.hpp>

namespace ek {

void gui_font_view(const font_t* font) {
    if (font) {
        if (ImGui::TreeNode(font, "Font")) {
            ImGui::LabelText("Glyphs", "%lu", font->map.size());
            ImGui::TreePop();
        }
    }
}

}