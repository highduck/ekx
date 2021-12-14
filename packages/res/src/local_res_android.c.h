#include <ek/local_res.h>
#include "local_res_sys.c.h"
#include <ek/app_native.h>

static void ek_local_res__AAsset_close(ek_local_res* lr) {
    if(lr->handle) {
        AAsset_close((AAsset*) lr->handle);
        lr->handle = NULL;
    }
    lr->buffer = NULL;
    lr->length = 0;
}

int ek_local_res_get_file_platform(const char* path, ek_local_res* lr) {
    AAsset* asset = AAssetManager_open(ek_android_assets(), path, AASSET_MODE_BUFFER);
    if (asset) {
        lr->handle = asset;
        lr->buffer = (uint8_t*)AAsset_getBuffer(asset);
        lr->length = AAsset_getLength(asset);
        lr->closeFunc = ek_local_res__AAsset_close;
        lr->status = 0;
        return 0;
    }
    lr->status = 1;
    return 1;
}
