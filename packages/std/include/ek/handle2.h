#ifndef EK_HANDLE_2_H
#define EK_HANDLE_2_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// calculate total bytes required to store `capacity` handles map
#define ek_handle2_get_alloc_size(capacity) ((capacity) * (sizeof(uint32_t) + sizeof(uint16_t)))

typedef struct ek_handle2_state {
    // dense[0] = u16 info[2]
    // info[0] = count (always non-zero) (in case if capacity is MAX and pool is FULL, so count == 0 and capacity == 0 after overflow)
    // info[1] = capacity (you should keep in mind - it will be zero in case of MAX capacity (0x10000)), there is no true zero capacity at all
    uint32_t* dense;
    uint16_t* sparse;
} ek_handle2_state;

typedef struct ek_handle2_data {
    void* data;
    uint32_t stride;
} ek_handle2_data;

void ek_handle2_init(ek_handle2_state h, uint32_t capacity);

int ek_handle2_count(ek_handle2_state h);

uint32_t ek_handle2_new(ek_handle2_state h);

bool ek_handle2_valid(ek_handle2_state h, uint32_t id);

void ek_handle2_destroy(ek_handle2_state h, uint32_t id, ek_handle2_data layout);

#ifdef __cplusplus
}
#endif


#ifdef EK_HANDLE_2_IMPLEMENTATION

#include <ek/assert.h>
#include <string.h>


#ifndef NDEBUG

void ek_handle2_check_state(ek_handle2_state h) {
    EK_ASSERT(h.dense);
    const uint16_t* info = (uint16_t*) h.dense;
    const uint32_t cnt = info[0] ? info[0] : 0x10000;
    const uint32_t cap = info[1] ? info[1] : 0x10000;
    EK_ASSERT(cnt > 0);
    EK_ASSERT(cap > 0);
    EK_ASSERT(cnt <= cap);

    EK_ASSERT(h.sparse);
    EK_ASSERT(h.sparse[0] == 0);
    for (uint32_t i = cnt; i < cap; ++i) {
        EK_ASSERT(h.sparse[i] == 0);
    }
}

#define EK_HANDLE_2_CHECK_STATE(h) do { ek_handle2_check_state(h); } while(0)
#else
#define EK_HANDLE_2_CHECK_STATE(h) do { } while(0)
#endif

void ek_handle2_init(ek_handle2_state h, uint32_t capacity) {
    EK_ASSERT(capacity > 0);
    EK_ASSERT(capacity - 1 <= 0xFFFFu);
    EK_ASSERT(h.dense);
    EK_ASSERT(h.sparse);
    for (uint32_t i = 0; i < capacity; ++i) {
        h.dense[i] = i;
    }

    uint16_t* info = (uint16_t*) h.dense;
    info[0] = 1;
    info[1] = (uint16_t) (capacity & 0xFFFFu);

    EK_HANDLE_2_CHECK_STATE(h);
}

#define UNLIKELY(x) __builtin_expect(!!(x), 0)

uint32_t ek_handle2_new(ek_handle2_state h) {
    EK_HANDLE_2_CHECK_STATE(h);
    uint16_t* info = (uint16_t*) h.dense;
    if (UNLIKELY(info[0] == info[1])) {
        // end of capacity: returns invalid id 0
        return 0;
    }

    const uint16_t slot_next = info[0]++;
    const uint32_t id = h.dense[slot_next];
    h.sparse[id & 0xFFFFu] = slot_next;
    return id;
}

int ek_handle2_count(ek_handle2_state h) {
    EK_HANDLE_2_CHECK_STATE(h);
    const uint16_t* info = (const uint16_t*) h.dense;
    const uint16_t num = info[0] - 1;
    return ((int)num) + 1;
}

bool ek_handle2_valid(ek_handle2_state h, uint32_t id) {
    EK_HANDLE_2_CHECK_STATE(h);
    const uint16_t slot_index = h.sparse[id & 0xFFFFu];
#ifndef NDEBUG
    // check if you use uninitialized handle, or sort of pool memory corruption
    const uint16_t* info = (uint16_t*) h.dense;
    EK_ASSERT(!info[0] || slot_index < info[0]);
#endif
    return h.dense[slot_index] == id;
}

void ek_handle2_destroy(ek_handle2_state h, uint32_t id, ek_handle2_data layout) {
    EK_HANDLE_2_CHECK_STATE(h);
    EK_ASSERT(ek_handle2_valid(h, id));
    const uint16_t current_index = id & 0xFFFFu;

    const uint32_t slot_index = h.sparse[current_index];
    h.sparse[current_index] = 0;

    uint16_t* info = (uint16_t*) h.dense;
    const uint32_t back_slot_index = --info[0];

    // get back slot index as `count - 1` and decrease count
    const uint32_t next_id = (((id >> 16u) + 1u) << 16u) | (uint32_t)current_index;

    if (slot_index == back_slot_index) {
        // last element, erase sparse lookup to invalid slot
        h.dense[slot_index] = next_id;
    } else {
        const uint16_t back_id = h.dense[back_slot_index];
        h.dense[slot_index] = back_id;
        h.dense[back_slot_index] = next_id;
        h.sparse[back_id & 0xFFFFu] = slot_index;
        if (layout.data) {
            memcpy((uint8_t*) layout.data + slot_index * layout.stride,
                   (uint8_t*) layout.data + back_slot_index * layout.stride, layout.stride);
        }
    }
}

#endif EK_HANDLE_2_IMPLEMENTATION

#endif // EK_HANDLE_2_H
