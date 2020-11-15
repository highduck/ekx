#pragma once

#include <vector>
#include <ecxx/ecxx.hpp>
#include <ek/math/box.hpp>
#include <ek/math/packed_color.hpp>

namespace ek {

/** Hierarchy **/
void guiHierarchyWindow(bool* p_open);

extern std::vector<ecs::entity> hierarchySelectionList;
extern std::string hierarchyFilterText;

void guiInspectorWindow(bool* p_open);

void guiStatsWindow(bool* p_open);


/** Widgets **/
struct TextLayerEffect;

void guiTextLayerEffect(TextLayerEffect& layer);

}

/** Utilities **/

namespace ImGui {

bool Color32Edit(const char* label, ek::argb32_t& argb);

bool EditRect(const char* label, float * xywh, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

}