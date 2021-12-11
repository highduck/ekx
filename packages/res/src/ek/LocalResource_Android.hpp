#pragma once

#include "LocalResource.hpp"
#include "LocalResource_System.hpp"
#include <ek/app_native.h>

namespace ek {

void closeAndroidAsset(LocalResource* lr) {
    if(lr->handle) {
        AAsset_close((AAsset*) lr->handle);
        lr->handle = nullptr;
    }
    lr->buffer = nullptr;
    lr->length = 0;
}

int getFile_platform(const char* path, LocalResource* lr) {
    auto* asset = AAssetManager_open(ek_android_assets(), path, AASSET_MODE_BUFFER);
    if (asset) {
        lr->handle = asset;
        lr->buffer = (uint8_t*)AAsset_getBuffer(asset);
        lr->length = AAsset_getLength(asset);
        lr->closeFunc = closeAndroidAsset;
        lr->status = 0;
        return 0;
    }
    lr->status = 1;
    return 1;
}

}