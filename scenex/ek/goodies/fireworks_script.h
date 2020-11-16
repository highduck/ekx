#pragma once

#include <ek/scenex/components/script.hpp>

namespace ek {

EK_DECL_SCRIPT_CPP(fireworks_script) {
public:

    void start() override;

    void update(float dt) override;

    void reset();

    bool enabled = true;

private:
    float timer_ = 1.0f;
};

}


