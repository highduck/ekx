#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/data/SGFile.hpp>
#include <ek/util/Res.hpp>
#include <optional>
#include <ek/timers.hpp>

namespace ek {

struct MovieClip {
    Res<SGFile> library_asset;
    std::string movie_data_symbol;
    const SGMovieData* data = nullptr;
    TimeLayer timer;

    [[nodiscard]]
    const SGMovieData* get_movie_data() const {
        const SGMovieData* result = data;
        if (!data && library_asset) {
            auto* symbol_data = library_asset->get(movie_data_symbol);
            if (symbol_data && symbol_data->movie) {
                result = &symbol_data->movie.value();
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

struct MovieClipTargetIndex {
    int32_t key = 0;
};

void goto_and_stop(ecs::EntityApi e, float frame);

}