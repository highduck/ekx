#include <ek/buf.h>
#include <ek/assert.h>

ek_buf_header_t* ek_buf_header(void* ptr) {
    EK_ASSERT_R2(ptr != 0);
    return ((ek_buf_header_t*) ptr) - 1;
}

uint32_t ek_buf_capacity(void* ptr) {
    // TODO: add test for empty array
    return ptr ? ek_buf_header(ptr)->capacity : 0;
}

uint32_t ek_buf_length(void* ptr) {
    // TODO: add test for empty array
    return ptr ? ek_buf_header(ptr)->length : 0;
}

bool ek_buf_full(void* ptr) {
    // TODO: add test for empty array
    if (ptr) {
        const ek_buf_header_t* hdr = ek_buf_header(ptr);
        return hdr->capacity == hdr->length;
    }
    return true;
}

bool ek_buf_empty(void* ptr) {
    return !ptr || !ek_buf_length(ptr);
}

void ek_buf_reset(void** ptr) {
    if (*ptr) {
        // destroy buffer content
        ek_buf_header_t* hdr = ek_buf_header(*ptr);
        free(hdr);
        *ptr = 0;
    }
}

void ek_buf_set_capacity(void** ptr, uint32_t newCapacity, uint32_t elementSize) {
    // delete case
    if (newCapacity == 0) {
        ek_buf_reset(ptr);
        return;
    }
    ek_buf_header_t* prevHeader = *ptr ? ek_buf_header(*ptr) : 0;
    if (prevHeader && prevHeader->capacity == newCapacity) {
        return;
    }

    // reallocate buffer content with different capacity
    ek_buf_header_t* hdr = (ek_buf_header_t*) realloc(prevHeader, sizeof(ek_buf_header_t) + newCapacity * elementSize);
    hdr->capacity = newCapacity;
    if (!prevHeader) {
        hdr->length = 0;
    }
    *ptr = (void*)(hdr + 1);
}

void ek_buf_set_size(void** buf, uint32_t elementSize, uint32_t len, uint32_t cap) {
    ek_buf_set_capacity(buf, cap, elementSize);
    if (*buf) {
        ek_buf_header(*buf)->length = len;
    }
}

void* ek_buf_add_(void* ptr, uint32_t elementSize) {
    EK_ASSERT_R2(ptr != 0);
    return (char*) ptr + elementSize * (ek_buf_header(ptr)->length++);
}

void* ek_buf_remove_(void* ptr, uint32_t i, uint32_t elementSize) {
    EK_ASSERT_R2(ptr != 0);
    ek_buf_header_t* hdr = ek_buf_header(ptr);
    EK_ASSERT_R2(i < hdr->length);
    --hdr->length;
    return (char*) ptr + elementSize * i;
}

void* ek_buf_pop_(void* ptr, uint32_t elementSize) {
    EK_ASSERT_R2(ptr != 0);
    ek_buf_header_t* hdr = ek_buf_header(ptr);
    EK_ASSERT_R2(hdr->length != 0);
    return (char*) ptr + elementSize * (--hdr->length);
}

