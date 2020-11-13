#pragma once

#include <ecxx/ecxx.hpp>
#include <functional>

namespace ek {

class drawable_text;

drawable_text& addText(ecs::entity e, const char* text);

ecs::entity createButton(const char* label, const std::function<void()>& fn);

}

