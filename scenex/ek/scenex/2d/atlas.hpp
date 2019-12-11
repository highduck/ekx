#pragma once

#include "sprite.hpp"
#include <ek/util/common_macro.hpp>
#include <ek/util/assets.hpp>
#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

namespace ek {

class atlas_t : private disable_copy_assign_t {
public:

    atlas_t();

    ~atlas_t();

    std::unordered_map<std::string, asset_t<sprite_t>> sprites;
    std::vector<asset_t<texture_t>> pages;
};

using load_atlas_callback = std::function<void(atlas_t*)>;
//atlas_t* load_atlas(const char* path, float scale_factor);
void load_atlas(const char* path, float scale_factor, const load_atlas_callback& callback);

}

