#pragma once

#include <ek/app/res.hpp>
#include <ek/app/impl/res_sys.hpp>
#include <ek/app/Platform.h>

namespace ek {

void get_resource_content_async(const char* path, const get_content_callback_func& callback) {
    auto* asset = AAssetManager_open(app::get_asset_manager(), path, AASSET_MODE_BUFFER);
    array_buffer buffer{};
    if (asset) {
        auto data = static_cast<const uint8_t*>(AAsset_getBuffer(asset));
        buffer.assign(data, data + AAsset_getLength(asset));
        AAsset_close(asset);
    } else {
        // TODO: better return error code
        //EKAPP_LOG("Asset file not found: %s", path);
    }
    callback(buffer);
}

}