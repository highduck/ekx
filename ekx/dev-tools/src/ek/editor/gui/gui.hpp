#pragma once

#include <vector>
#include <string>
#include <ecxx/ecxx.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

class Editor;

void guiEditor(Editor& inspector);

/** Hierarchy **/
void guiHierarchyWindow(bool* p_open);

extern std::vector<ecs::EntityApi> hierarchySelectionList;
extern std::string hierarchyFilterText;

void guiInspectorWindow(bool* p_open);

void guiStatsWindow(bool* p_open);

void guiResourcesViewWindow(bool* p_open);

/** Widgets **/
struct TextLayerEffect;

void guiTextLayerEffect(TextLayerEffect& layer);

class Sprite;

class Atlas;

class Font;

class SceneFactory;

struct ParticleDecl;

void gui_sprite_view(const Sprite* sprite);

void gui_atlas_view(const Atlas* atlas);

void gui_font_view(const Font* font);

void gui_sg_file_view(const SceneFactory* library);

void gui_particles(ParticleDecl* decl);
}

/** Utilities **/

namespace ImGui {

bool Color32Edit(const char* label, ek::argb32_t& argb);

bool EditRect(const char* label, float * xywh, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

}