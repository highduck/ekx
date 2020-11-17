#include <ek/scenex/2d/AAtlas.hpp>
#include <ek/editor/imgui/imgui.hpp>
#include <ek/graphics/texture.hpp>

namespace ek {

void gui_sprite_view(const Sprite* sprite) {
    if (sprite) {
        ImGui::PushID(sprite);
        auto rc = sprite->rect;
        auto uv0 = sprite->tex.position;
        auto uv1 = sprite->tex.right_bottom();
        if (sprite->texture) {
            void* tex_id = reinterpret_cast<void*>(sprite->texture->handle());
            if (sprite->rotated) {
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
        ImGui::PopID();
    } else {
        ImGui::TextColored(ImColor{1.0f, 0.0f, 0.0f}, "Sprite is NULL");
    }
}

void gui_atlas_view(const Atlas* atlas) {
    if (atlas) {
        if (ImGui::TreeNode(atlas, "Atlas")) {
            for (const auto& page : atlas->pages) {
                ImGui::Image(reinterpret_cast<void*>(page->handle()), ImVec2{100.0f, 100.0f});
            }
            for (const auto&[id, spr] : atlas->sprites) {
                ImGui::LabelText("Sprite", "%s", id.c_str());
                gui_sprite_view(spr.get());
            }
            ImGui::TreePop();
        }
    }
}

}