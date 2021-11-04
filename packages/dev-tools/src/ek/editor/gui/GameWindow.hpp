#pragma once

#include "EditorWindow.hpp"

namespace ek {

class GameWindow : public EditorWindow {
public:

    bool paused = false;
    float timeScale = 1.0f;
    bool profiler = false;

    GameWindow() {
        name = "GameWindow";
        title = ICON_FA_GAMEPAD " Game###GameWindow";
        fullFrame = true;
    }

    ~GameWindow() override = default;

    void onDraw() override;
    void onLoad(const pugi::xml_node& xml) override;
    void onSave(pugi::xml_node& xml) override;
};

}