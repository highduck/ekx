#pragma once

namespace ImGui {

void HelpMarker(const char* desc);

bool Color32Edit(const char* label, ek::argb32_t& argb);

bool EditRect(const char* label, float * xywh, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

bool ToolbarButton(const char* label, bool active, const char* tooltip);

}

namespace ek {

struct TextLayerEffect;

void guiTextLayerEffect(TextLayerEffect& layer);

void guiSprite(const Sprite& sprite);

void guiFont(const Font& font);

std::string getDebugNodePath(ecs::EntityApi e);

}