#pragma once

#include "EditorWindow.hpp"

namespace ek {

class SceneWindow : public EditorWindow {
public:

    SceneWindow() {
        name = "SceneWindow";
        title = ICON_FA_GLOBE " Scene###SceneWindow";
    }

    ~SceneWindow() override = default;

    void onDraw() override;
};

}