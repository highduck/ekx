#include "static_resources.hpp"

#include <cassert>
#include <cstdio>
#include <cstring>
#include <ek/logger.hpp>

#if defined(__EMSCRIPTEN__)

#include <emscripten/fetch.h>
#include <unordered_map>

#endif

#if defined(__ANDROID__)

#include <ek/android.hpp>

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
    array_buffer buffer{};
    if(asset) {
        const void *asset_buffer = AAsset_getBuffer(asset);
        auto asset_size = static_cast<size_t>(AAsset_getLength(asset));
        buffer = array_buffer{asset_size};
        memcpy(buffer.data(), asset_buffer, asset_size);
        AAsset_close(asset);
    }
    else {
        EK_ERROR("Asset file not found: %s", path);
    }
    return buffer;
}

#elif defined(__linux__) || defined(__EMSCRIPTEN__)

#define EK_STATIC_RESOURCES_SYS

array_buffer get_resource_content(const char* path) {
    return get_content(path);
}

#endif

#if defined(__EMSCRIPTEN__)

static uint32_t next_callback_id_ = 0;
static std::unordered_map<uint32_t, std::function<void(array_buffer)>> fetch_callbacks_{};

void get_resource_content_async(const char* path, std::function<void(array_buffer)> callback) {

    const auto callback_id = next_callback_id_++;
    fetch_callbacks_[callback_id] = std::move(callback);

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.userData = reinterpret_cast<void*>(callback_id);
    attr.onerror = [](emscripten_fetch_t* fetch) {
        EK_ERROR("Downloading %s failed, HTTP failure status code: %d.\n", fetch->url, fetch->status);
        const auto user_callback_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(fetch->userData));
        const auto& user_callback = fetch_callbacks_[user_callback_id];
        emscripten_fetch_close(fetch);
        user_callback(array_buffer{});
        fetch_callbacks_.erase(user_callback_id);
    };
    attr.onsuccess = [](emscripten_fetch_t* fetch) {
        EK_DEBUG("Finished downloading %llu bytes from URL %s.\n", fetch->numBytes, fetch->url);
        const auto user_callback_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(fetch->userData));
        const auto& user_callback = fetch_callbacks_[user_callback_id];
        array_buffer buffer{
                reinterpret_cast<const uint8_t*>(fetch->data),
                static_cast<size_t>(fetch->numBytes)
        };
        emscripten_fetch_close(fetch);
        user_callback(std::move(buffer));
        fetch_callbacks_.erase(user_callback_id);
    };
    emscripten_fetch(&attr, path);
}

#else

void get_resource_content_async(const char* path, std::function<void(array_buffer)> callback) {
    callback(get_resource_content(path));
}

#endif

}