#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/2d/font.hpp>
#include <ek/fonts/export_font.hpp>
#include <ek/rtfont/ttf_font.hpp>

namespace ek {

void gui_font_view(const font_t* font) {
    if (font) {
        if (ImGui::TreeNode(font, "Font")) {
            switch (font->getType()) {
                case FontType::Bitmap: {
                    auto* bm = static_cast<const BitmapFont*>(font->getImpl());
                    ImGui::LabelText("Glyphs", "%lu", bm->map.size());
                }
                    break;
                case FontType::TrueType: {
                    //auto* ttf = static_cast<const TTFFont*>(font->getImpl());
                }
                    break;
            }
            ImGui::TreePop();
        }
    }
}

}