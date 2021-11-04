#pragma once

#include "EditorWindow.hpp"

namespace ek {

class MemoryProfiler : public EditorWindow {
public:
    MemoryProfiler() {
        name = "MemoryProfiler";
        title = ICON_FA_MEMORY " Memory###MemoryProfiler";
    }

    ~MemoryProfiler() override = default;

    void onDraw() override;
};

}