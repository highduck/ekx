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
    ImGui::Text("Page Count: %lu", pagesCount);
    static float pageScale = 0.25f;
    ImGui::SliderFloat("Scale", &pageScale, 0.0f, 1.0f);
    for (int i = 0; i < pagesCount; ++i) {
        const auto* page = asset.getPageTexture(i);
        if (page) {
            ImGui::Text("Page #%d", i);
            const auto width = pageScale * (float) page->desc.width;
            const auto height = pageScale * (float) page->desc.height;
            ImGui::Image((void*) (uintptr_t) page->image.id, ImVec2{width, height});
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
        const auto* page = asset.pages[i].get();
        if (page) {
            ImGui::Text("Page #%d", i);
            const auto width = pageScale * (float) page->desc.width;
            const auto height = pageScale * (float) page->desc.height;
            ImGui::Image((void*) (uintptr_t) page->image.id, ImVec2{width, height});
        } else {
            ImGui::TextDisabled("Page #%d", i);
        }
    }

    for (const auto&[id, spr] : asset.sprites) {
        const auto* sprite = spr.get();
        if (sprite) {
            ImGui::TextUnformatted(id.c_str());
            guiSprite(*sprite);
        } else {
            ImGui::TextDisabled("Sprite %s is null", id.c_str());
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
        for (auto& sceneName : asset.scenes) {
            ImGui::TextUnformatted(sceneName.c_str());
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("##linkages-list", "Linkages (%u)", static_cast<uint32_t>(asset.linkages.size()))) {
        for (auto[k, v] : asset.linkages) {
            auto* node = sg_get(asset, v);
            if(node) {
                if (ImGui::TreeNode(node, "%s -> %s", k.c_str(), v.c_str())) {
                    ImGui::TextDisabled("todo:");
                    ImGui::TreePop();
                }
            }
            else {
                ImGui::TextDisabled("%s -> %s (not found)", k.c_str(), v.c_str());
            }
        }
        ImGui::TreePop();
    }
}

template<typename T>
void drawAssetsListByType(const char* typeName) {
    auto count = static_cast<uint32_t>(Res<T>::map().size());
    char buff[128];
    sprintf(buff, "%s (%u)###%s", typeName, count, typeName);
    if (ImGui::BeginTabItem(buff)) {
        for (const auto&[key, value]: Res<T>::map()) {
            Res<T> asset{key};
            const T* content = asset.get();
            if (content) {
                if (ImGui::TreeNode(key.c_str())) {
                    drawAssetItem<T>(*content);
                    ImGui::TreePop();
                }
            } else {
                ImGui::TextDisabled("%s", key.c_str());
            }
        }
        ImGui::EndTabItem();
    }
}

void ResourcesWindow::onDraw() {
    if (ImGui::BeginTabBar("res_by_type", 0)) {
        drawAssetsListByType<Material3D>("Material");
        drawAssetsListByType<StaticMesh>("Mesh");
        drawAssetsListByType<Atlas>("Atlas");
        drawAssetsListByType<Sprite>("Sprite");
        drawAssetsListByType<Font>("Font");
        drawAssetsListByType<SGFile>("Scenes 2D");
        drawAssetsListByType<DynamicAtlas>("Dynamic Atlas");
        drawAssetsListByType<ParticleDecl>("Particle");
        ImGui::EndTabBar();
    }
}

}