#pragma once

namespace ek::flash {

enum class blend_mode_t;

blend_mode_t& operator<<(blend_mode_t& r, const char* str);

}


