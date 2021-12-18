// TODO: we still need C++ for stateful callback functions

#if defined(__EMSCRIPTEN__)

#include <ek/local_res.hpp>
#include <emscripten.h>

namespace ek {

void closeMemoryFile(ek_local_res* lr) {
    free(lr->buffer);
    lr->buffer = nullptr;
    lr->length = 0;
}

}

extern "C" {

extern int ek_fetch_open(const char* pURL);
extern int ek_fetch_load(int id);
extern int ek_fetch_close(int id);
extern int ek_fetch_read(int id, void* pBuffer, int toRead, int offset);

static ek::get_content_callback_func ek_local_res_web_callbacks_cxx[64]{};
static ek_local_res_callback ek_local_res_web_callbacks[64];
static void* ek_local_res_web_callbacks_userdata[64];

EMSCRIPTEN_KEEPALIVE void _ekfs_onComplete(int id, int err, int size) {
    using namespace ek;
    ek_local_res lr{};
    lr.status = err;
    if (err == 0) {
        lr.buffer = (uint8_t*)malloc(size);
        lr.length = size;
        lr.status = 0;
        lr.closeFunc = closeMemoryFile;
        // ... move to persistent LR objects
        lr.userdata = ek_local_res_web_callbacks_userdata[id];

        const int numRead = ek_fetch_read(id, lr.buffer, size, 0);
        if (numRead != size) {
            lr.status = 1;
            ek_local_res_close(&lr);
        }
    }

    if(ek_local_res_web_callbacks[id]) {
        ek_local_res_web_callbacks[id](&lr);
        ek_local_res_web_callbacks[id] = NULL;
    }
    if (ek_local_res_web_callbacks_cxx[id]) {
        ek_local_res_web_callbacks_cxx[id](lr);
        ek_local_res_web_callbacks_cxx[id] = nullptr;
    }

    ek_fetch_close(id);
}

void ek_local_res_load(const char* path, ek_local_res_callback callback, void* userdata) {
    const int id = ek_fetch_open(path);
    if (id == 0) {
        ek_local_res lr = {0};
        lr.status = 1;
        lr.userdata = userdata;
        callback(&lr);
        return;
    }
    ek_local_res_web_callbacks_cxx[id] = nullptr;
    ek_local_res_web_callbacks[id] = callback;
    ek_local_res_web_callbacks_userdata[id] = userdata;
    int result = ek_fetch_load(id);
    if (result != 0) {
        ek_local_res lr = {0};
        lr.status = 1;
        lr.userdata = userdata;
        callback(&lr);

        ek_local_res_web_callbacks[id] = NULL;
        ek_local_res_web_callbacks_userdata[id] = NULL;
        ek_fetch_close(id);
    }
}

}

namespace ek {

void get_resource_content_async(const char* path, get_content_callback_func callback) {
    const auto id = ek_fetch_open(path);
    if (id == 0) {
        if (callback) {
            callback({});
        }
        return;
    }
    ek_local_res_web_callbacks_cxx[id] = std::move(callback);
    ek_local_res_web_callbacks[id] = nullptr;
    int result = ek_fetch_load(id);
    if (result != 0) {
        if (ek_local_res_web_callbacks_cxx[id]) {
            ek_local_res_web_callbacks_cxx[id]({});
        }
        ek_local_res_web_callbacks_cxx[id] = nullptr;
        ek_fetch_close(id);
    }
}

}

#endif