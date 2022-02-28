#include <ek/nep.h>
#include <ek/assert.h>

#include <msgpack/msgpack_all.c>
//#include <emscripten.h>

//extern uint32_t js_call(void* in_ptr, uint32_t in_size, uint32_t call_id);

static call_t* calls[1024] = {0};
static uint32_t call_next_id = 0;

call_t* create_call(call_callback_t callback, void* userdata) {
    call_t* call = (call_t*)calloc(1, sizeof(call_t));
    call->id = call_next_id;
    call->callback = callback;
    call->userdata = userdata;
    call_next_id = (call_next_id + 1) & (1024 - 1);
    calls[call->id] = call;
    return call;
}

void destroy_call(call_t* call) {
    calls[call->id] = 0;
    free(call);
}

void* serialize_request(msgpack_object args, uint32_t* out_size) {
    static msgpack_sbuffer sbuf = {0};
    sbuf.size = 0;

    msgpack_packer pk;
    msgpack_packer_init(&pk, &sbuf, msgpack_sbuffer_write);
    msgpack_pack_object(&pk, args);

    *out_size = sbuf.size;
    return sbuf.data;
}

uint32_t deserialize_request(void* data, uint32_t size, msgpack_object* out) {
    static msgpack_unpacker unpacker = {0};
    msgpack_unpacked unpacked = {0};
    msgpack_unpacker_init(&unpacker, MSGPACK_UNPACKER_INIT_BUFFER_SIZE);
    msgpack_unpacker_reset(&unpacker);
    if (msgpack_unpacker_buffer_capacity(&unpacker) < size) {
        bool result = msgpack_unpacker_reserve_buffer(&unpacker, size);
        if (!result) {
            /* Memory allocation error. */
        }
    }
    memcpy(msgpack_unpacker_buffer(&unpacker), data, size);
    msgpack_unpacker_buffer_consumed(&unpacker, size);
    msgpack_unpack_return ret = msgpack_unpacker_next(&unpacker, &unpacked);
    switch(ret) {
        case MSGPACK_UNPACK_SUCCESS:
            *out = unpacked.data;
            return 0;
        case MSGPACK_UNPACK_CONTINUE:
            /* cheking capacity, reserve buffer, copy additional data to the buffer, */
            /* notify consumed buffer size, then call msgpack_unpacker_next(&unp, &und) again */
            return 2;
        case MSGPACK_UNPACK_PARSE_ERROR:
            /* Error process */
            return 1;
    }
    return 3;
}

void call_native(void* data, uint32_t size, uint32_t call_id) {
//    js_call(data, size, call_id);
}

void send_call(call_t* call, msgpack_object args) {
    call->pending += 1;

    uint32_t size = 0;
    uint8_t* data = serialize_request(args, &size);
    call_native(data, size, call->id);
}

void handle_call_result(call_t* call, void* data, uint32_t size) {
    EK_ASSERT(call->pending != 0);
    call->pending -= 1;
    if(deserialize_request(data, size, &call->result)) {
        EK_ASSERT(false && "failed to decode result");
    }
    call->callback(call);
    if(call->pending == 0) {
        destroy_call(call);
    }
}
//
//EMSCRIPTEN_KEEPALIVE
//void js_resolve_call(uint32_t call_id, void* data, uint32_t size) {
//    handle_call_result(calls[call_id], data, size);
//}
