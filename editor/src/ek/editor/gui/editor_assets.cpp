#include "editor_assets.hpp"

#include <ek/editor/imgui/imgui.hpp>
#include <ek/util/assets.hpp>
#include <ek/graphics/program.hpp>
#include <ek/scenex/3d/material_3d.hpp>
#include <ek/scenex/3d/static_mesh.hpp>
#include <ek/scenex/2d/atlas.hpp>
#include <ek/scenex/text/font.hpp>
#include <ek/scenex/data/sg_data.hpp>

namespace ek {

template<typename T>
void on_editor_debug_asset(const asset_t<T>& asset) {

}

template<>
void on_editor_debug_asset<static_mesh_t>(const asset_t<static_mesh_t>& asset) {
    ImGui::Text("Indices: %i", asset->indices_count);
}

template<>
void on_editor_debug_asset<material_3d>(const asset_t<material_3d>& asset) {
    auto m = asset.get_mutable();
    ImGui::ColorEdit3("Ambient", m->ambient.data_);
    ImGui::ColorEdit3("Diffuse", m->diffuse.data_);
    ImGui::ColorEdit3("Specular", m->specular.data_);
    ImGui::ColorEdit3("Emission", m->emission.data_);
    ImGui::DragFloat("Shininess", &m->shininess, 0.1f, 1.0f, 128.0f);
    ImGui::DragFloat("Roughness", &m->roughness, 0.01f, 0.001f, 1.0f);
}

template<>
void on_editor_debug_asset<sg_file>(const asset_t<sg_file>& asset) {
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
        for (const auto&[key, value]: asset_t<T>::map()) {
            asset_t<T> asset{key};
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

void do_editor_debug_runtime_assets() {
    ImGui::Begin("Debug Assets");

    do_editor_debug_runtime_asset_list<graphics::program_t>("Program");
    do_editor_debug_runtime_asset_list<material_3d>("Material");
    do_editor_debug_runtime_asset_list<static_mesh_t>("Mesh");
    do_editor_debug_runtime_asset_list<atlas_t>("Atlas");
    do_editor_debug_runtime_asset_list<sprite_t>("Sprite");
    do_editor_debug_runtime_asset_list<Font>("Font");
    do_editor_debug_runtime_asset_list<sg_file>("Scenes 2D");

    ImGui::End();
}

}
