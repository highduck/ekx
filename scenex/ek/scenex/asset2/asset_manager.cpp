#include "asset_manager.hpp"
#include "asset_object.hpp"

#include <ek/util/logger.hpp>
#include <ek/util/path.hpp>
#include <ek/math/common.hpp>

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

asset_manager_t::asset_manager_t() = default;

asset_manager_t::~asset_manager_t() {
    clear();
    for (auto* res : resolvers) {
        delete res;
    }
}

void asset_manager_t::load_all() {
    for (auto asset : assets) {
        asset->load();
    }
}

void asset_manager_t::unload_all() {
    for (auto asset : assets) {
        asset->unload();
    }
}

void asset_manager_t::clear() {
    for (auto* asset : assets) {
        asset->unload();
        delete asset;
    }
    assets.clear();
}

void asset_manager_t::set_scale_factor(float scale) {
    auto new_uid = get_scale_uid(scale);
    scale_factor = math::clamp(scale, 1.0f, 4.0f);
    if (scale_uid != new_uid) {
        scale_uid = new_uid;
        // todo: maybe better naming `update`?
        load_all();
    }
}

void asset_manager_t::add_resolver(asset_type_resolver_t* resolver) {
    resolver->project_ = this;
    resolvers.push_back(resolver);
}

asset_object_t* asset_manager_t::add_from_type(const std::string& type, const std::string& path) {
    for (const auto* resolver : resolvers) {
        asset_object_t* asset = resolver->create_for_type(type, path);
        if (asset) {
            asset->project_ = this;
            assets.push_back(asset);
            return asset;
        }
    }
    EK_ERROR("Can't resolve asset for file: %s", path.c_str());
    return nullptr;
}

void asset_manager_t::add_file(const std::string& path) {
    for (const auto* resolver : resolvers) {
        asset_object_t* asset = resolver->create_from_file(path);
        if (asset) {
            asset->project_ = this;
            assets.push_back(asset);
            return;
        }
    }
    EK_WARN("Can't resolve asset for file %s", path.c_str());
}

bool asset_manager_t::is_assets_ready() const {
    for (auto* asset : assets) {
        if (!asset->ready_) {
            return false;
        }
    }
    return true;
}

}