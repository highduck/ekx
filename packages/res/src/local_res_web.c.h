#include <emscripten.h>

// js lib
extern int ek_fetch_open(const char* pURL);

extern int ek_fetch_load(int id);

extern int ek_fetch_close(int id);

extern int ek_fetch_read(int id, void* pBuffer, int toRead, int offset);

static void ek_local_res_close_fetch(ek_local_res* lr) {
    free(lr->buffer);
    lr->buffer = NULL;
    lr->length = 0;
    if (lr->handle != 0) {
        ek_fetch_close((int) (uintptr_t) lr->handle);
        lr->handle = 0;
    }
}

static ek_local_res ek_local_res_list[32];

EMSCRIPTEN_KEEPALIVE void _ekfs_onComplete(int id, int err, int size) {
    ek_local_res* lr = ek_local_res_list + id;
    lr->status = err;
    if (err == 0) {
        lr->buffer = (uint8_t*) malloc(size);
        lr->length = size;
        lr->status = 0;
        const int numRead = ek_fetch_read(id, lr->buffer, size, 0);
        if (numRead != size) {
            lr->status = 1;
            ek_local_res_close(lr);
        }
    }

    lr->callback(lr);
}

void ek_local_res_load(const char* path, ek_local_res_callback callback, void* userdata) {
    const int id = ek_fetch_open(path);
    if (id == 0) {
        callback(&(ek_local_res) {
                .status = 1,
                .userdata = userdata
        });
        return;
    }
    ek_local_res_list[id] = (ek_local_res) {
            .callback = callback,
            .userdata = userdata,
            .handle = (void*) (uintptr_t) id,
            .closeFunc = ek_local_res_close_fetch,
            .buffer = NULL,
            .length = 0
    };
    int result = ek_fetch_load(id);
    if (result != 0) {
        ek_local_res_list[id].status = 1;
        callback(ek_local_res_list + id);
    }
}
