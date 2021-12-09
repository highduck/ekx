#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/scenex/SceneFactory.hpp>
#include <ek/util/Res.hpp>
#include <ek/ds/String.hpp>
#include "../base/TimeLayer.hpp"

namespace ek {

struct MovieClip {
    Res<SGFile> library_asset;
    String movie_data_symbol;
    const SGMovieData* data = nullptr;
    TimeLayer timer;

    [[nodiscard]]
    const SGMovieData* get_movie_data() const {
        const SGMovieData* result = data;
        if (!data && library_asset) {
            auto* symbol_data = sg_get(*library_asset, movie_data_symbol.c_str());
            if (symbol_data && !symbol_data->movie.empty()) {
                result = &symbol_data->movie[0];
            }
        }
        return result;
    }

    float time = 0.0f;
    bool playing = true;
    float fps = 24.0f;

    void trunc_time() {
        const auto* dat = get_movie_data();
        if (dat) {
            const auto max = static_cast<float>(dat->frames);
            if (time >= max) {
                time -= max * truncf(time / max);
            }
            if (time < 0) {
                time = 0;
            }
        }
    }

    static void updateAll();
};

ECX_TYPE(13, MovieClip);

struct MovieClipTargetIndex {
    int32_t key = 0;
};

ECX_TYPE(30, MovieClipTargetIndex);

void goto_and_stop(ecs::EntityApi e, float frame);

}