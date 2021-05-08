#include <ek/scenex/2d/Atlas.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/graphics/graphics.hpp>

namespace ek {

void gui_sprite_view(const Sprite& sprite) {
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

}