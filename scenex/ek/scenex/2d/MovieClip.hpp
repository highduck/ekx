#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <ek/util/assets.hpp>
#include <optional>
#include <ek/scenex/TimeLayer.hpp>

namespace ek {

struct MovieClip {
    Res<sg_file> library_asset;
    std::string movie_data_symbol;
    const sg_movie_data* data = nullptr;
    TimeLayer timer;

    [[nodiscard]]
    const sg_movie_data* get_movie_data() const {
        const sg_movie_data* result = data;
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

struct movie_target_keys {
    int key_animation = 0;
};

void goto_and_stop(ecs::entity e, float frame);

}