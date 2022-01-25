#pragma once

#include "Asset.hpp"

#include <ek/log.h>
#include <ek/assert.h>
#include <ek/util/Path.hpp>
#include <ek/math.h>

namespace ek {

inline uint8_t get_scale_uid(float scale) {
    if (scale > 3.0f) {
        return 4;
    } else if (scale > 2.0f) {
        return 3;
    } else if (scale > 1.0f) {
        return 2;
    }
    return 1;
}

void AssetManager::load_all() {
    for (auto asset : assets) {
        asset->load();
    }
}

void AssetManager::unload_all() {
    for (auto asset : assets) {
        asset->unload();
    }
}

void AssetManager::clear() {
    unload_all();

    for (auto* asset : assets) {
        delete asset;
    }
    assets.clear();
}

void AssetManager::set_scale_factor(float scale) {
    auto new_uid = get_scale_uid(scale);
    scale_factor = clamp(scale, 1, 4);
    if (scale_uid != new_uid) {
        log_debug("asset manager: content scale changed to %d%%", (int) (100 * scale_factor));
        scale_uid = new_uid;
        // todo: maybe better naming `update`?
        load_all();
    }
}

void AssetManager::add(Asset* asset) {
    EK_ASSERT(asset);
    asset->manager_ = this;
    assets.push_back(asset);
}

bool AssetManager::is_assets_ready() const {
    for (auto* asset : assets) {
        if (asset->state != AssetState::Ready) {
            return false;
        }
    }
    return true;
}

}