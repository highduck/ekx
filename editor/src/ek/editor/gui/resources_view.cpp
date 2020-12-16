#include <ek/editor/imgui/imgui.hpp>
#include <ek/util/Res.hpp>
#include <ek/graphics/program.hpp>
#include <ek/graphics/texture.hpp>
#include <ek/scenex/3d/Material3D.hpp>
#include <ek/scenex/3d/static_mesh.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>

namespace ek {

template<typename T>
void on_editor_debug_asset(const Res<T>& asset) {

}

template<>
void on_editor_debug_asset<static_mesh_t>(const Res<static_mesh_t>& asset) {
    ImGui::Text("Indices: %i", asset->indices_count);
}

template<>
void on_editor_debug_asset<DynamicAtlas>(const Res<DynamicAtlas>& asset) {
    auto pagesCount = asset->pages_.size();
    ImGui::Text("Page Size: %d x %d", asset->pageWidth, asset->pageHeight);
    ImGui::Text("Page Count: %lu", pagesCount);
    for (size_t i = 0; i < pagesCount; ++i) {
        ImGui::Text("Page #%lu", i);
        auto* page = asset->getPageTexture(i);
        ImGui::Image(reinterpret_cast<void*>(page->handle()), ImVec2{100.0f, 100.0f});
    }
}

template<>
void on_editor_debug_asset<Material3D>(const Res<Material3D>& asset) {
    auto m = asset.get_mutable();
    ImGui::ColorEdit3("Ambient", m->ambient.data());
    ImGui::ColorEdit3("Diffuse", m->diffuse.data());
    ImGui::ColorEdit3("Specular", m->specular.data());
    ImGui::ColorEdit3("Emission", m->emission.data());
    ImGui::DragFloat("Shininess", &m->shininess, 0.1f, 1.0f, 128.0f);
    ImGui::DragFloat("Roughness", &m->roughness, 0.01f, 0.001f, 1.0f);
}

template<>
void on_editor_debug_asset<sg_file>(const Res<sg_file>& asset) {
    auto m = asset.get_mutable();
    for (auto[k, v] : m->linkages) {
        if (ImGui::TreeNode("%s %s", k.c_str(), v.c_str())) {

            ImGui::TreePop();
        }
    }
}

template<typename T>
void do_editor_debug_runtime_asset_list(const std::string& type_name) {
    if (ImGui::TreeNode(type_name.c_str())) {
        for (const auto&[key, value]: Res<T>::map()) {
            Res<T> asset{key};
            if (asset) {
                if (ImGui::TreeNode(key.c_str())) {
                    on_editor_debug_asset<T>(asset);
                    ImGui::TreePop();
                }
            } else {
                ImGui::TextDisabled("%s", key.c_str());
            }
        }
        ImGui::TreePop();
    }
}

void guiResourcesViewWindow(bool* p_open) {
    if (ImGui::Begin("Resources", p_open)) {
        do_editor_debug_runtime_asset_list<graphics::program_t>("Program");
        do_editor_debug_runtime_asset_list<Material3D>("Material");
        do_editor_debug_runtime_asset_list<static_mesh_t>("Mesh");
        do_editor_debug_runtime_asset_list<Atlas>("Atlas");
        do_editor_debug_runtime_asset_list<Sprite>("Sprite");
        do_editor_debug_runtime_asset_list<Font>("Font");
        do_editor_debug_runtime_asset_list<sg_file>("Scenes 2D");
        do_editor_debug_runtime_asset_list<DynamicAtlas>("Dynamic Atlas");
    }

    ImGui::End();
}

}
