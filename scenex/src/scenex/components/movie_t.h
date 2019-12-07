#pragma once

#include <scenex/data/sg_data.h>
#include <ek/util/assets.hpp>
#include <optional>

namespace scenex {

struct movie_t {
    ek::asset_t<sg_file> library_asset;
    std::string movie_data_symbol;
    const sg_movie_data* data = nullptr;

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
        auto* data = get_movie_data();
        if (data && time >= static_cast<float>(data->frames)) {
            time -= data->frames * static_cast<int>(time / data->frames);
        }
    }
};

struct movie_target_keys {
    int key_animation = 0;
    int key_layer = 0;
};

}