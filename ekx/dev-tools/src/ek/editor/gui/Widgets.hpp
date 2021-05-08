#pragma once

namespace ImGui {

void HelpMarker(const char* desc);

bool Color32Edit(const char* label, ek::argb32_t& argb);

bool EditRect(const char* label, float * xywh, float v_speed = 1.0f, const char* format = "%.3f", float power = 1.0f);

}