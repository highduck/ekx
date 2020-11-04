#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/text/font.hpp>
#include <ek/scenex/text/truetype_font.hpp>
#include <ek/fonts/export_font.hpp>

namespace ek {

void gui_font_view(const Font* font) {
    if (font) {
        if (ImGui::TreeNode(font, "Font")) {
            switch (font->getFontType()) {
                case FontType::Bitmap: {
                    auto* bm = reinterpret_cast<const BitmapFont*>(font->getImpl());
                    ImGui::LabelText("Font Type", "Bitmap");
                    ImGui::LabelText("Glyphs", "%lu", bm->map.size());
                }
                    break;
                case FontType::TrueType: {
                    auto* ttf = reinterpret_cast<const TrueTypeFont*>(font->getImpl());
                    ImGui::LabelText("Font Type", "TrueType");
                    ImGui::LabelText("Glyphs", "%lu", ttf->map_.size());
                }
                    break;
            }
            ImGui::TreePop();
        }
    }
}

}