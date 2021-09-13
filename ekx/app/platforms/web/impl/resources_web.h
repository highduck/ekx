#pragma once

#include <ek/app/res.hpp>
#include <ek/app/impl/res_sys.hpp>

#include <emscripten/fetch.h>
#include <unordered_map>

namespace ek {

static uint32_t next_callback_id_ = 0;
static std::unordered_map<uint32_t, get_content_callback_func> fetch_callbacks_{};

void get_resource_content_async(const char* path, const get_content_callback_func& callback) {

    const auto callback_id = next_callback_id_++;
    fetch_callbacks_[callback_id] = callback;

    emscripten_fetch_attr_t attr;
    emscripten_fetch_attr_init(&attr);
    strcpy(attr.requestMethod, "GET");
    attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;
    attr.userData = reinterpret_cast<void*>(callback_id);
    attr.onerror = [](emscripten_fetch_t* fetch) {
        //EK_ERROR("Downloading %s failed, HTTP failure status code: %d", fetch->url, fetch->status);
        const auto user_callback_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(fetch->userData));
        const auto& user_callback = fetch_callbacks_[user_callback_id];
        emscripten_fetch_close(fetch);
        user_callback({});
        fetch_callbacks_.erase(user_callback_id);
    };
    attr.onsuccess = [](emscripten_fetch_t* fetch) {
        //EK_TRACE("Finished downloading %llu bytes from URL %s", fetch->numBytes, fetch->url);
        const auto user_callback_id = static_cast<uint32_t>(reinterpret_cast<uintptr_t>(fetch->userData));
        const auto& user_callback = fetch_callbacks_[user_callback_id];
        array_buffer buffer{fetch->data, fetch->data + fetch->numBytes};
        emscripten_fetch_close(fetch);
        user_callback(std::move(buffer));
        fetch_callbacks_.erase(user_callback_id);
    };
    emscripten_fetch(&attr, path);
}

}