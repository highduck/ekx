#pragma once

#include <ek/scenex/base/Script.hpp>

namespace ek {

EK_DECL_SCRIPT_CPP(fireworks_script) {
public:

    void start() override;

    void update(float dt) override;

    void reset();

    bool enabled = false;

private:
    float timer_ = 1.0f;
};

EK_TYPE_INDEX_T(ScriptBase, fireworks_script, 6);

}


