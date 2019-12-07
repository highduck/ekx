#pragma once

#include "sprite.hpp"
#include <ek/util/common_macro.hpp>
#include <ek/util/assets.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace scenex {

class atlas_t : private ek::disable_copy_assign_t {
public:

    atlas_t();

    ~atlas_t();

    std::unordered_map<std::string, ek::asset_t<sprite_t>> sprites;
    std::vector<ek::asset_t<ek::texture_t>> pages;
};

//atlas_t* load_atlas(const char* path, float scale_factor);
void load_atlas(const char* path, float scale_factor, std::function<void(atlas_t*)> callback);

}

