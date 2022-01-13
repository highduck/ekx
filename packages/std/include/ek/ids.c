#include <ek/ids.h>
#include <ek/assert.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NDEBUG

static void ids_sanitize_state(ids_t h) {
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

#define IDS_SANITIZE_STATE(h) do { ids_sanitize_state(h); } while(0)
#else
#define IDS_SANITIZE_STATE(h) do { } while(0)
#endif

void ids_init(ids_t h, uint32_t capacity) {
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

    IDS_SANITIZE_STATE(h);
}

int ids_count(ids_t h) {
    IDS_SANITIZE_STATE(h);
    const uint16_t* info = (const uint16_t*) h.dense;
    const uint16_t num = info[0] - 1;
    return ((int) num) + 1;
}

uint32_t id_new(ids_t h) {
    IDS_SANITIZE_STATE(h);
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

bool id_valid(ids_t h, uint32_t id) {
    IDS_SANITIZE_STATE(h);
    const uint16_t slot_index = h.sparse[id & 0xFFFFu];
#ifndef NDEBUG
    // check if you use uninitialized ID, or sort of pool memory corruption
    const uint16_t* info = (uint16_t*) h.dense;
    EK_ASSERT(!info[0] || slot_index < info[0]);
#endif
    return h.dense[slot_index] == id;
}

void id_destroy(ids_t h, uint32_t id, id_data_t layout) {
    IDS_SANITIZE_STATE(h);
    EK_ASSERT(id_valid(h, id));
    const uint16_t current_index = id & 0xFFFFu;

    const uint32_t slot_index = h.sparse[current_index];
    h.sparse[current_index] = 0;

    uint16_t* info = (uint16_t*) h.dense;
    const uint32_t back_slot_index = --info[0];

    // get back slot index as `count - 1` and decrease count
    const uint32_t next_id = (((id >> 16u) + 1u) << 16u) | (uint32_t) current_index;

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

#ifdef __cplusplus
}
#endif
