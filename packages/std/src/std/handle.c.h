#include <ek/handle.h>
#include <ek/assert.h>
#include <string.h>

#define HANDLE_BITS_IDX (16u)
#define HANDLE_BITS_GEN (8u)
#define HANDLE_MASK_IDX ((1u << HANDLE_BITS_IDX) - 1u)
#define HANDLE_MASK_GEN (((1u << HANDLE_BITS_GEN) - 1u) << HANDLE_BITS_IDX)

void ek_handles_init(ek_handles_desc h) {
    EK_ASSERT(h.sparse);
    EK_ASSERT(h.dense);
    EK_ASSERT(h.num > 1);
    EK_ASSERT(h.num <= 0x10000);
    // reserve 0 index as invalid,
    // store next free index in 0 index slot
    // sparse[0] = next_free_index
#pragma nounroll
    for (int i = 1; i < h.num; ++i) {
        // to fit all indices we need to clamp capacity to (0xFFFF - 1)
        // next   : 0 2 ... 0xFFFF |      0
        // indexes: 0 1 ... 0xFFFE | 0xFFFF
        *h.sparse = i;
        ++h.sparse;
    }
    // set last free index to 0 (out of handles)
    *h.sparse = 0;


    // store dense count in [0] element
    h.dense[0] = 1;
}

uint32_t ek_handles_new(ek_handles_desc h) {
    EK_ASSERT(h.sparse);
    EK_ASSERT(h.generations);
    EK_ASSERT(h.dense);

    // alloc next free index
    uint16_t* free_list = h.sparse;
    const uint16_t index = free_list[0];
    // unlikely
    if (index == 0) {
        return 0;
    }
    const uint16_t next_index = free_list[index];
    free_list[0] = next_index;

    // alloc next free data slot at the end
    uint32_t slot_index = h.dense[0]++ /* dense array count */;

    // wire sparse to dense
    free_list[index] = slot_index;
    // wire dense to sparse
    h.dense[slot_index] = index;

    return (uint32_t) index | ((uint32_t) h.generations[index] << 16u);
}

bool ek_handles_valid(ek_handles_desc h, uint32_t id) {
    EK_ASSERT(h.sparse);
    EK_ASSERT(h.generations);
    EK_ASSERT(h.dense);
    EK_ASSERT((id & HANDLE_MASK_IDX) < h.num);
    if (id == 0) {
        return false;
    }
    const uint16_t index = id & HANDLE_MASK_IDX;
    const uint16_t slot_index = h.sparse[index];
    // gen match && dense index match sparse
    const uint32_t c_index = (uint32_t) h.dense[slot_index];
    const uint32_t c_gen = (uint32_t) h.generations[index];
    const uint32_t valid_id = c_index | (c_gen << 16u);
    return valid_id == id;
}

void ek_handles_destroy(ek_handles_desc h, uint32_t id, ek_handles_data_layout layout) {
    EK_ASSERT(ek_handles_valid(h, id));
    const uint32_t index = id & HANDLE_MASK_IDX;
    ++h.generations[index];
    h.sparse[index] = h.sparse[0];
    h.sparse[0] = index;
    uint16_t slot_index = h.dense[index];
    uint16_t count = h.dense[0];
    uint16_t slot_back = count - 1;
    if (slot_index == slot_back) {
        // last element
        h.dense[slot_index] = 0;
    } else {
        const uint16_t back_index = h.dense[slot_back];
        h.sparse[back_index] = slot_index;
        h.dense[slot_index] = back_index;
        h.dense[slot_back] = 0;
        if(layout.data) {
            memcpy((uint8_t*) layout.data + slot_index * layout.stride,
                   (uint8_t*) layout.data + slot_back * layout.stride, layout.stride);
        }
    }
}

uint16_t ek_handles__get_count(ek_handles_desc h) {
    EK_ASSERT(h.dense);
    return h.dense[0];
}

uint16_t ek_handles__get_slot_index(ek_handles_desc h, uint16_t index) {
    EK_ASSERT(h.sparse);
    return h.sparse[index];
}
