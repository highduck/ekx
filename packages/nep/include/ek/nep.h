#ifndef EK_NEP_H
#define EK_NEP_H

#include <stdint.h>
#include <stdbool.h>
#include <msgpack.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct call_t call_t;

typedef void(*call_callback_t)(call_t* call);

struct call_t {
    uint32_t id;
    call_callback_t callback;
    void* userdata;
    // is completed (status & 1) == 0
    uint32_t status;
    // available only in callback scope
    msgpack_object result;

    uint32_t pending;
};

void send_call(call_t* call, msgpack_object args);

call_t* create_call(call_callback_t callback, void* userdata);

void destroy_call(call_t* call);

#ifdef __cplusplus
}
#endif

#endif //EK_NEP_H
