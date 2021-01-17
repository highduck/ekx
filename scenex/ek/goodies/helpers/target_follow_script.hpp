#pragma once

#include <ek/math/vec.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ecxx/ecxx.hpp>

namespace ek {

EK_DECL_SCRIPT_CPP(target_follow_script) {
public:

    enum class integration_mode {
        Exp = 0,
        Steps = 1,
        None = 2,
    };

    float2 offset{};
    float2 target{};
    ecs::EntityRef target_entity{};

    float k = 0.1f;
    float fixed_frame_rate = 60.0f;

    int n = 1;
    int counter = 0;
    float time_accum = 0.0f;

    int frame = 0;
    int reset_in = 100;

    integration_mode integration = integration_mode::Exp;

    void update(float dt) override;

    void gui_gizmo() override;

    void gui_inspector() override;
};


//struct SyncPosition {
//
//};

}


