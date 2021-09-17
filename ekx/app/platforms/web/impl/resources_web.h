#pragma once

#include <ek/app/res.hpp>
#include <ek/app/impl/res_sys.hpp>
#include <emscripten.h>

namespace ek {

inline get_content_callback_func fetch_callbacks_[64]{};

}

extern "C" {

extern int ek_fetch_open(const char* pURL);
extern int ek_fetch_load(int id);
extern int ek_fetch_close(int id);
extern int ek_fetch_read(int id, void* pBuffer, int toRead, int offset);

EMSCRIPTEN_KEEPALIVE void _ekfs_onComplete(int id, int err, int size) {
    std::vector<uint8_t> buffer;
    if (err == 0) {
        buffer.resize(size);
        const int numRead = ek_fetch_read(id, buffer.data(), size, 0);
        if (numRead != size) {
            buffer.resize(0);
            buffer.shrink_to_fit();
        }
    }

    if (ek::fetch_callbacks_[id]) {
        ek::fetch_callbacks_[id](std::move(buffer));
        ek::fetch_callbacks_[id] = nullptr;
    }

    ek_fetch_close(id);
}

}

namespace ek {

void get_resource_content_async(const char* path, const get_content_callback_func& callback) {
    const auto id = ek_fetch_open(path);
    if (id == 0) {
        if (callback) {
            callback({});
        }
        return;
    }
    fetch_callbacks_[id] = callback;
    int result = ek_fetch_load(id);
    if (result != 0) {
        if (fetch_callbacks_[id]) {
            fetch_callbacks_[id]({});
        }
        fetch_callbacks_[id] = nullptr;
        ek_fetch_close(id);
    }
}

}