#pragma once

namespace ek {

struct TimeLayerState {
    float scale = 1.0f;
    float dt = 0.0f;
    float total = 0.0f;
};

struct TimeLayer {

    TimeLayer() = default;

    explicit TimeLayer(unsigned char id_) :
            id{id_} {

    }

    TimeLayerState* operator->();

    const TimeLayerState* operator->() const;

    static TimeLayer Root;
    static TimeLayer Game;
    static TimeLayer HUD;
    static TimeLayer UI;

    static void updateTimers(float raw);

private:
    unsigned char id = 0;
};
}


