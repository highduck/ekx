#ifndef EK_IO_H
#define EK_IO_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// just bunch of functions to operate memory stream read/write

#ifdef __cplusplus
extern "C" {
#endif
    
typedef struct io_t {
    int pos;
    int cap;
    void* data;
} io_t;

void io_alloc(io_t* io, int cap);

void io_free(io_t* io);

void io_ensure(io_t* io, int bytes_to_write);

void io_push(io_t* io, const void* data, int size);

void io_write_i32(io_t* io, int32_t v);

void io_write_u32(io_t* io, uint32_t v);

void io_write_u16(io_t* io, uint16_t v);

void io_write_u8(io_t* io, uint8_t v);

void io_write_f32(io_t* io, float v);

// ( chars size | ...size bytes string chars | '\0' )
void io_write_string(io_t* io, const char* str, int32_t size);

#ifdef __cplusplus
};
#endif

#endif // EK_IO_H
