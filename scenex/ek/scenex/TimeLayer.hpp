#pragma once

namespace ek {



struct TimeLayer {
    struct State {
        float scale = 1.0f;
        float dt = 0.0f;
        float total = 0.0f;
    };

    TimeLayer() = default;

    explicit TimeLayer(unsigned char id_) :
            id{id_} {

    }

    State* operator->();

    const State* operator->() const;

    static TimeLayer Root;
    static TimeLayer Game;
    static TimeLayer HUD;
    static TimeLayer UI;

    static void updateTimers(float raw);

private:
    unsigned char id = 0;
};
}


