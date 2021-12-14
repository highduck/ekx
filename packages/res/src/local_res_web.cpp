// TODO: we still need C++ for stateful callback functions

#if defined(__EMSCRIPTEN__)

#include <ek/local_res.hpp>
#include <emscripten.h>

namespace ek {

inline get_content_callback_func fetch_callbacks_[64]{};

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

EMSCRIPTEN_KEEPALIVE void _ekfs_onComplete(int id, int err, int size) {
    using namespace ek;
    ek_local_res lr{};
    lr.status = err;
    if (err == 0) {
        lr.buffer = (uint8_t*)malloc(size);
        lr.length = size;
        lr.status = 0;
        lr.closeFunc = closeMemoryFile;
        const int numRead = ek_fetch_read(id, lr.buffer, size, 0);
        if (numRead != size) {
            lr.status = 1;
            ek_local_res_close(&lr);
        }
    }

    if (fetch_callbacks_[id]) {
        fetch_callbacks_[id](lr);
        fetch_callbacks_[id] = nullptr;
    }

    ek_fetch_close(id);
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
    fetch_callbacks_[id] = std::move(callback);
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

#endif