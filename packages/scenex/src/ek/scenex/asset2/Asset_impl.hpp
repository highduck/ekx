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

AssetManager::AssetManager() = default;

AssetManager::~AssetManager() {
    clear();
    for (auto* res : resolvers) {
        delete res;
    }
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
    for (auto* asset : assets) {
        asset->unload();
        delete asset;
    }
    assets.clear();
}

void AssetManager::set_scale_factor(float scale) {
    auto new_uid = get_scale_uid(scale);
    scale_factor = clamp(scale, 1, 4);
    if (scale_uid != new_uid) {
        scale_uid = new_uid;
        // todo: maybe better naming `update`?
        load_all();
    }
}

void AssetManager::add_resolver(AssetTypeResolver* resolver) {
    resolver->manager = this;
    resolvers.push_back(resolver);
}

Asset* AssetManager::add_from_type(const void* data, uint32_t size) {
    for (const auto* resolver : resolvers) {
        Asset* asset = resolver->create_for_type(data, size);
        if (asset) {
            asset->manager_ = this;
            assets.push_back(asset);
            return asset;
        }
    }
    EK_ERROR("Can't resolve asset from buffer...");
    return nullptr;
}

Asset* AssetManager::add_file(const char* path, const char* type) {
    for (const auto* resolver : resolvers) {
        Asset* asset = resolver->create_from_file(path, type);
        if (asset) {
            asset->manager_ = this;
            assets.push_back(asset);
            return asset;
        }
    }
    EK_WARN("Can't resolve asset [%s] from file: %s", type, path);
    return nullptr;
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