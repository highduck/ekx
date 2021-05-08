#include <ek/editor/imgui/imgui.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/text/TrueTypeFont.hpp>
#include <ek/scenex/text/BitmapFont.hpp>

namespace ek {

void gui_font_view(const Font& font) {
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