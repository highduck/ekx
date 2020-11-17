#pragma once

#include <vector>
#include <ecxx/ecxx.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

class Editor;

void guiEditor(Editor& inspector);

void guiBuildWindow(bool* p_open);

/** Hierarchy **/
void guiHierarchyWindow(bool* p_open);

extern std::vector<ecs::entity> hierarchySelectionList;
extern std::string hierarchyFilterText;

void guiInspectorWindow(bool* p_open);

void guiStatsWindow(bool* p_open);

void guiResourcesViewWindow(bool* p_open);

class editor_project_t;

void guiAssetsViewWindow(bool* p_open, editor_project_t& project);

/** Widgets **/
struct TextLayerEffect;

void guiTextLayerEffect(TextLayerEffect& layer);

class Sprite;

class Atlas;

class Font;

struct sg_file;

struct editor_project_t;

struct particle_decl;

class editor_asset_t;

void gui_sprite_view(const Sprite* sprite);

void gui_atlas_view(const Atlas* atlas);

void gui_font_view(const Font* font);

void gui_sg_file_view(const sg_file* library);

void gui_asset_object_controls(editor_asset_t* asset);

void gui_particles(particle_decl* decl);
}

/** Utilities **/

namespace ImGui {

bool Color32Edit(const char* label, ek::argb32_t& argb);

bool EditRect(const char* label, float * xywh, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

}