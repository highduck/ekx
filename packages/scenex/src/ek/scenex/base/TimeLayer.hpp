#pragma once

namespace ek {

struct TimeLayer {

    inline static constexpr float DeltaTimeMax = 0.3f;

    struct State {
        float scale = 1.0f;
        float dt = 0.0f;
        float total = 0.0f;
        float pad_ = 0.0f;
    };

    static State Layers[4];

    TimeLayer() = default;

    explicit TimeLayer(unsigned char id_) :
            id{id_} {

    }

    inline State* operator->() const {
        return Layers + id;
    }

    static TimeLayer Root;
    static TimeLayer Game;
    static TimeLayer HUD;
    static TimeLayer UI;

    static void updateTimers(float raw);

private:
    unsigned char id = 0;
};

}