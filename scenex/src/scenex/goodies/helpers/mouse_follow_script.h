#pragma once

#include <scenex/components/script_t.h>

namespace scenex {

class mouse_follow_script : public script {
public:
    void update(float dt) override;
    void gui_gizmo() override;
    void gui_inspector() override;
};

}


