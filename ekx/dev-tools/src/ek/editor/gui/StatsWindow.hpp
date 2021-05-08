#pragma once

#include "EditorWindow.hpp"

namespace ek {

class StatsWindow : public EditorWindow {
public:

    StatsWindow() {
        name = "StatsWindow";
        title = ICON_FA_TACHOMETER_ALT " Stats###StatsWindow";
    }

    ~StatsWindow() override = default;

    void onDraw() override;
};

}