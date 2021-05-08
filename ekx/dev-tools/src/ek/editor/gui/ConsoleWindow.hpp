#pragma once

#include "EditorWindow.hpp"

namespace ek {

class ConsoleWindow : public EditorWindow {
public:

    ConsoleWindow() {
        name = "ConsoleWindow";
        // ICON_FA_BUG
        // ICON_FA_TERMINAL
        title = ICON_FA_ENVELOPE_OPEN_TEXT " Console###ConsoleWindow";
    }

    ~ConsoleWindow() override = default;

    void onDraw() override;
};

}