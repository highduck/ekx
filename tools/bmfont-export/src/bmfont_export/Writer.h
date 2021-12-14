#pragma once

#include <cstdlib>
#include <cstring>

namespace {

typedef struct bytes_writer {
    int pos;
    int cap;
    void* data;
} bytes_writer;

void bytes_writer_alloc(bytes_writer* writer, int cap) {
    writer->pos = 0;
    writer->cap = cap;
    writer->data = malloc(cap);
}

void bytes_writer_ensure(bytes_writer* writer, int bytes_to_write) {
    int l = writer->pos + bytes_to_write;
    int cap = writer->cap;
    if (l > cap) {
        while (l > cap) {
            cap <<= 1;
        }
        writer->data = (uint8_t*) realloc(writer->data, cap);
        writer->cap = cap;
    }
}

void bytes_writer_free(bytes_writer* writer) {
    free(writer->data);
    writer->data = NULL;
}

void bytes_writer_push(bytes_writer* writer, const void* data, int size) {
    bytes_writer_ensure(writer, size);
    memcpy((char*)writer->data + writer->pos, data, size);
    writer->pos += size;
}

void bytes_write_i32(bytes_writer* writer, int32_t v) {
    bytes_writer_push(writer, &v, sizeof(v));
}

void bytes_write_u32(bytes_writer* writer, uint32_t v) {
    bytes_writer_push(writer, &v, sizeof(v));
}

void bytes_write_u64(bytes_writer* writer, uint64_t v) {
    bytes_writer_push(writer, &v, sizeof(v));
}

void bytes_write_u16(bytes_writer* writer, uint16_t v) {
    bytes_writer_push(writer, &v, sizeof(v));
}

void bytes_write_u8(bytes_writer* writer, uint8_t v) {
    bytes_writer_push(writer, &v, sizeof(v));
}

void bytes_write_f32(bytes_writer* writer, float v) {
    bytes_writer_push(writer, &v, sizeof(v));
}

// ( chars size | ...size bytes string chars | '\0' )
void bytes_write_string(bytes_writer* writer, const char* str, int32_t size) {
    bytes_writer_ensure(writer, size + 4 + 1);
    memcpy((char*)writer->data + writer->pos, &size, sizeof(size));
    writer->pos += sizeof(size);
    memcpy((char*)writer->data + writer->pos, str, size + 1);
    writer->pos += size;
    *((char*)writer->data + writer->pos) = '\0';
    ++writer->pos;
}
}
