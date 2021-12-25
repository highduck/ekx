#pragma once

#include "ResourcesWindow.hpp"

#include <ek/util/Res.hpp>
#include <ek/scenex/3d/Material3D.hpp>
#include <ek/scenex/3d/StaticMesh.hpp>
#include <ek/scenex/2d/Atlas.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/2d/DynamicAtlas.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/scenex/text/Font.hpp>
#include <ek/scenex/text/TrueTypeFont.hpp>
#include <ek/scenex/text/BitmapFont.hpp>

namespace ek {

template<typename T>
void drawAssetItem(const T& asset) {

}

template<>
void drawAssetItem<StaticMesh>(const StaticMesh& asset) {
    ImGui::Text("Indices: %i", asset.indices_count);
}

template<>
void drawAssetItem<DynamicAtlas>(const DynamicAtlas& asset) {
    auto pagesCount = asset.pages_.size();
    ImGui::Text("Page Size: %d x %d", asset.pageWidth, asset.pageHeight);
    ImGui::Text("Page Count: %u", pagesCount);
    static float pageScale = 0.25f;
    ImGui::SliderFloat("Scale", &pageScale, 0.0f, 1.0f);
    for (int i = 0; i < pagesCount; ++i) {
        const sg_image page = asset.getPageTexture(i);
        if (page.id) {
            ImGui::Text("Page #%d", i);
            const auto info = sg_query_image_info(page);
            const auto width = pageScale * (float) info.width;
            const auto height = pageScale * (float) info.height;
            ImGui::Image((void*) (uintptr_t) page.id, ImVec2{width, height});
        } else {
            ImGui::TextDisabled("Page #%d", i);
        }
    }
}

template<>
void drawAssetItem<Atlas>(const Atlas& asset) {
    auto pagesCount = asset.pages.size();
    ImGui::Text("Page Count: %u", pagesCount);
    static float pageScale = 0.25f;
    ImGui::SliderFloat("Scale", &pageScale, 0.0f, 1.0f);
    for (int i = 0; i < pagesCount; ++i) {
        const sg_image page = ek_texture_reg_get(asset.pages[i]);
        if (page.id) {
            ImGui::Text("Page #%d", i);
            const auto info = sg_query_image_info(page);
            const auto width = pageScale * (float) info.width;
            const auto height = pageScale * (float) info.height;
            ImGui::Image((void*) (uintptr_t) page.id, ImVec2{width, height});
        } else {
            ImGui::TextDisabled("Page #%d", i);
        }
    }

    for (const auto& spr: asset.sprites) {
        const auto* sprite = spr.get();
        const auto* id = spr.getID();
        if (sprite) {
            ImGui::TextUnformatted(id);
            guiSprite(*sprite);
        } else {
            ImGui::TextDisabled("Sprite %s is null", id);
        }
        ImGui::Separator();
    }
}

template<>
void drawAssetItem<Sprite>(const Sprite& asset) {
    guiSprite(asset);
}

template<>
void drawAssetItem<Font>(const Font& asset) {
    guiFont(asset);
}

template<>
void drawAssetItem<Material3D>(const Material3D& asset) {
    auto& m = const_cast<Material3D&>(asset);
    ImGui::ColorEdit3("Ambient", m.ambient.data());
    ImGui::ColorEdit3("Diffuse", m.diffuse.data());
    ImGui::ColorEdit3("Specular", m.specular.data());
    ImGui::ColorEdit3("Emission", m.emission.data());
    ImGui::DragFloat("Shininess", &m.shininess, 0.1f, 1.0f, 128.0f);
    ImGui::DragFloat("Roughness", &m.roughness, 0.01f, 0.001f, 1.0f);
}

template<>
void drawAssetItem<SGFile>(const SGFile& asset) {
    if (ImGui::TreeNode("##scene-list", "Scenes (%u)", asset.scenes.size())) {
        for (auto& sceneName: asset.scenes) {
            ImGui::TextUnformatted(sceneName.c_str());
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("##linkages-list", "Linkages (%u)", static_cast<uint32_t>(asset.linkages.size()))) {
        for (auto& info: asset.linkages) {
            auto* node = sg_get(asset, info.linkage.c_str());
            if (node) {
                if (ImGui::TreeNode(node, "%s -> %s", info.name.c_str(), info.linkage.c_str())) {
                    ImGui::TextDisabled("todo:");
                    ImGui::TreePop();
                }
            } else {
                ImGui::TextDisabled("%s -> %s (not found)", info.name.c_str(), info.linkage.c_str());
            }
        }
        ImGui::TreePop();
    }
}

template<typename T>
void drawAssetsListByType() {
    FixedArray<ResourceDB::Slot*, 1024> list;
    // TODO:
    //for (auto& it: ResourceDB::getMap) {
//        if (it.second.key.type == TypeIndex<T>::value) {
//            list.push_back(&it.second);
//        }
    //}

    const char* typeName = TypeName<T>::value;
    const auto count = list.size();
    char buff[128];
    sprintf(buff, "%s (%u)###%s", typeName, count, typeName);
    if (ImGui::BeginTabItem(buff)) {
        for (const auto& slot: list) {
            const T* content = (const T*) slot->content;
            if (content) {
                if (ImGui::TreeNode(slot->name.c_str())) {
                    drawAssetItem<T>(*content);
                    ImGui::TreePop();
                }
            } else {
                ImGui::TextDisabled("%s", slot->name.c_str());
            }
        }
        ImGui::EndTabItem();
    }
}

void ResourcesWindow::onDraw() {
    // TODO: somehow generic draw manual registries
    // drawAssetsListByType<Texture>();

    if (ImGui::BeginTabBar("res_by_type", 0)) {
        drawAssetsListByType<Shader>();
        drawAssetsListByType<Sprite>();
        drawAssetsListByType<Atlas>();
        drawAssetsListByType<DynamicAtlas>();
        drawAssetsListByType<Font>();
        drawAssetsListByType<SGFile>();
        drawAssetsListByType<AudioResource>();
        drawAssetsListByType<ParticleDecl>();
        drawAssetsListByType<Material3D>();
        drawAssetsListByType<StaticMesh>();

        ImGui::EndTabBar();
    }
}

}