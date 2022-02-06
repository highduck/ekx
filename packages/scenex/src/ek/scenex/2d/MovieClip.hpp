#pragma once

#include <ecxx/ecxx_fwd.hpp>
#include <ek/format/SGFile.hpp>
#include <ek/scenex/SceneFactory.hpp>

#include <ek/ds/String.hpp>
#include <ekx/app/time_layers.h>

namespace ek {

struct MovieClip {
    const SGMovieData* data = nullptr;
    float time = 0.0f;
    float fps = 24.0f;
    TimeLayer timer = 0;
    bool playing = true;

    static void updateAll();

    void trunc_time() {
        if (data) {
            const float max = (float)data->frames;
            if (time >= max) {
                time -= max * truncf(time / max);
            }
            if (time < 0) {
                time = 0;
            }
        }
    }
};

struct MovieClipTargetIndex {
    int32_t key = 0;
};

void goto_and_stop(ecs::EntityApi e, float frame);

}