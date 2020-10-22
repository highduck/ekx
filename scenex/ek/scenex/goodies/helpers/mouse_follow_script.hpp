#pragma once

#include <ek/scenex/components/script.hpp>

namespace ek {

EK_DECL_SCRIPT_CPP(mouse_follow_script) {
public:
    void update(float dt) override;
    void gui_gizmo() override;
    void gui_inspector() override;
};

}

