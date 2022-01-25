#pragma once

//#include <ek/debug.hpp>
//#define IM_ASSERT_USER_ERROR(_EXP,_MSG) log_warn(_MSG)

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <implot.h>

static inline ImVec2 operator-(const ImVec2& lhs, const ImVec2& rhs) { return ImVec2(lhs.x - rhs.x, lhs.y - rhs.y); }

namespace ImGui {

}