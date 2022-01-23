#include <ek/io.h>

// just bunch of functions to operate memory stream read/write

#ifdef __cplusplus
extern "C" {
#endif

void io_alloc(io_t* io, int cap) {
    io->pos = 0;
    io->cap = cap;
    io->data = malloc(cap);
}

void io_ensure(io_t* io, int bytes_to_write) {
    int l = io->pos + bytes_to_write;
    int cap = io->cap;
    if (l > cap) {
        while (l > cap) {
            cap <<= 1;
        }
        io->data = (uint8_t*) realloc(io->data, cap);
        io->cap = cap;
    }
}

void io_free(io_t* io) {
    free(io->data);
    io->data = NULL;
}

void io_push(io_t* io, const void* data, int size) {
    io_ensure(io, size);
    memcpy((char*)io->data + io->pos, data, size);
    io->pos += size;
}

void io_write_i32(io_t* io, int32_t v) {
    io_push(io, &v, sizeof(v));
}

void io_write_u32(io_t* io, uint32_t v) {
    io_push(io, &v, sizeof(v));
}

void io_write_u16(io_t* io, uint16_t v) {
    io_push(io, &v, sizeof(v));
}

void io_write_u8(io_t* io, uint8_t v) {
    io_push(io, &v, sizeof(v));
}

void io_write_f32(io_t* io, float v) {
    io_push(io, &v, sizeof(v));
}

// ( chars size | ...size bytes string chars | '\0' )
void io_write_string(io_t* io, const char* str, int32_t size) {
    io_ensure(io, size + 4 + 1);
    memcpy((char*)io->data + io->pos, &size, sizeof(size));
    io->pos += sizeof(size);
    memcpy((char*)io->data + io->pos, str, size + 1);
    io->pos += size;
    *((char*)io->data + io->pos) = '\0';
    ++io->pos;
}

#ifdef __cplusplus
};
#endif
