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

/// canvas stats
void draw_buffer_chain_stats(const char* name, ek_canvas_buffers* buffers) {
    for (int line = 0; line < 4; ++line) {
        int c = 0;
        for (int i = 0; i < CANVAS_BUFFERS_MAX_COUNT; ++i) {
            if (buffers->lines[line][i].id == 0) {
                i = CANVAS_BUFFERS_MAX_COUNT;
            } else {
                ++c;
            }
        }
        ImGui::Text("%s[%d] count %d", name, line, c);
    }
}

void draw_canvas_stats() {
    ImGui::Text("size: %lu", sizeof canvas);
    ImGui::Text("vb chain size: %lu", sizeof canvas.vbs);
    ImGui::Text("ib chain size: %lu", sizeof canvas.ibs);
    ImGui::Text("vb mem size: %lu", sizeof canvas.vertex);
    ImGui::Text("ib mem size: %lu", sizeof canvas.index);
    draw_buffer_chain_stats("VB", &canvas.vbs);
    draw_buffer_chain_stats("IB", &canvas.ibs);
}

/// assets
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
        const sg_image page = asset.get_page_image(i);
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
        const sg_image page = REF_RESOLVE(res_image, asset.pages[i]);
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

    for (const auto spr: asset.sprites) {
        const auto* sprite = &REF_RESOLVE(res_sprite, spr);
        ImGui::Text("%s (GID: %u)", hsp_get(res_sprite.names[spr]), spr);
        if (sprite->state & SPRITE_LOADED) {
            guiSprite(sprite);
        } else {
            ImGui::TextDisabled("Unloaded");
        }
        ImGui::Separator();
    }
}

void draw_sprite_info(void* asset) {
    guiSprite((const Sprite*)asset);
}

template<>
void drawAssetItem<Font>(const Font& asset) {
    guiFont(asset);
}

template<>
void drawAssetItem<Material3D>(const Material3D& asset) {
    auto& m = const_cast<Material3D&>(asset);
    ImGui::ColorEdit3("Ambient", m.ambient.data);
    ImGui::ColorEdit3("Diffuse", m.diffuse.data);
    ImGui::ColorEdit3("Specular", m.specular.data);
    ImGui::ColorEdit3("Emission", m.emission.data);
    ImGui::DragFloat("Shininess", &m.shininess, 0.1f, 1.0f, 128.0f);
    ImGui::DragFloat("Roughness", &m.roughness, 0.01f, 0.001f, 1.0f);
}

template<>
void drawAssetItem<SGFile>(const SGFile& asset) {
    if (ImGui::TreeNode("##scene-list", "Scenes (%u)", asset.scenes.size())) {
        for (auto& sceneName: asset.scenes) {
            ImGui::Text("%u %s", sceneName, hsp_get(sceneName));
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("##linkages-list", "Linkages (%u)", static_cast<uint32_t>(asset.linkages.size()))) {
        for (auto& info: asset.linkages) {
            auto* node = sg_get(asset, info.linkage);
            if (ImGui::TreeNode(node, "%u %s -> %u %s", info.name, hsp_get(info.name), info.linkage,
                                hsp_get(info.linkage))) {
                if (node) {
                    ImGui::TextUnformatted("todo:");
                } else {
                    ImGui::TextDisabled("Not found");
                };
                ImGui::TreePop();
            }
        }
        ImGui::TreePop();
    }
}

template<typename T>
void drawAssetsListByType() {
    const char* typeName = TypeName<T>::value;
    auto& list = ResourceDB::list(TypeIndex<T>::value);
    const auto count = list.size();
    char buff[128];
    sprintf(buff, "%s (%u)###%s", typeName, count, typeName);
    if (ImGui::BeginTabItem(buff)) {
        for (const auto& slot: list) {
            const T* content = (const T*) slot.content;
            const char* name = hsp_get(slot.name);
            if (content) {
                if (ImGui::TreeNode(&slot, "%s (0x%08X)", name, slot.name)) {
                    drawAssetItem<T>(*content);
                    ImGui::TreePop();
                }
            } else {
                ImGui::TextDisabled("%s (0x%08X)", name, slot.name);
            }
        }
        ImGui::EndTabItem();
    }
}


//template<typename T>
void draw_rr_items(const char* type_name, rr_man_t* rr, void (* fn)(void* item)) {
    char buff[128];
    sprintf(buff, "%s (%u / %u)###%s", type_name, rr->num, rr->max, type_name);
    if (ImGui::BeginTabItem(buff)) {
        uint8_t* item_data = (uint8_t*) rr->data;
        for (uint32_t i = 0; i < rr->num; ++i) {
            string_hash_t name = rr->names[i];
            ImGui::BeginDisabled(!fn);
            if (ImGui::TreeNode(item_data, "%s (0x%08X)", hsp_get(name), name)) {
                if (fn) {
                    fn(item_data);
                }
                ImGui::TreePop();
            }
            ImGui::EndDisabled();
            item_data += rr->data_size;
        }
        ImGui::EndTabItem();
    }
}

void ResourcesWindow::onDraw() {
    if (ImGui::BeginTabBar("res_by_type", 0)) {
        draw_rr_items("image", &res_image.rr, 0);
        draw_rr_items("shader", &res_shader.rr, 0);
        draw_rr_items("sprite", &res_sprite.rr, draw_sprite_info);
        draw_rr_items("particle", &res_particle.rr, 0);
        draw_rr_items("audio", &res_audio.rr, 0);

        drawAssetsListByType<Atlas>();
        drawAssetsListByType<DynamicAtlas>();
        drawAssetsListByType<Font>();
        drawAssetsListByType<SGFile>();
        drawAssetsListByType<ParticleDecl>();
        drawAssetsListByType<Material3D>();
        drawAssetsListByType<StaticMesh>();

        ImGui::EndTabBar();
    }
}

}