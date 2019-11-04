#include "static_resources.hpp"

#include <cassert>

#if defined(__ANDROID__)
#include <platform/ek_android.h>
#endif

namespace ek {

array_buffer get_content(const char* path) {
    assert(path);
    array_buffer buffer;
    auto* stream = fopen(path, "rb");
    if (stream) {
        fseek(stream, 0, SEEK_END);
        buffer = array_buffer{
                static_cast<size_t>(ftell(stream))
        };
        fseek(stream, 0, SEEK_SET);

        fread(buffer.data(), buffer.size(), 1u, stream);

        if (ferror(stream) != 0) {
            buffer = array_buffer{};
        }

        fclose(stream);
    }
    return buffer;
}

bool save_content(const char* path, const array_buffer& buffer) {
    assert(path);
    auto* stream = fopen(path, "wb");
    if (stream) {
        fwrite(buffer.data(), buffer.size(), 1u, stream);
        auto err = ferror(stream);
        fclose(stream);
        return err == 0;
    }
    return false;
}

#if defined(__ANDROID__)

array_buffer get_resource_content(const char* path) {
    auto* asset = AAssetManager_open(::ek::android::get_asset_manager(), path,
                                     AASSET_MODE_STREAMING);
    const void* asset_buffer = AAsset_getBuffer(asset);
    auto asset_size = static_cast<size_t>(AAsset_getLength(asset));
    array_buffer buffer{asset_size};
    memcpy(buffer.data(), asset_buffer, asset_size);
    AAsset_close(asset);
    return buffer;
}

#elif (defined(__linux__) || defined(__EMSCRIPTEN__))

#define EK_STATIC_RESOURCES_SYS

array_buffer get_resource_content(const char* path) {
    return get_content(path);
}

#endif

}
