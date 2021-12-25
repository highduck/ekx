#include "temp_res_man.h"

typedef struct ek_image_reg {
    // allocated count
    uint32_t size;
    // next free index
    uint32_t next;
    uint32_t ids[128];
    uint16_t count;
    uint16_t sparse_to_dense[128];
    uint16_t dense_to_sparse[128];
    sg_image data[128];
    ek_image_reg_name names[128];
} ek_image_reg;

ek_image_reg ek_image_reg_;

ek_pool* ek_pool_alloc(uint32_t capacity) {
    EK_ASSERT(capacity > 1);
    EK_ASSERT(capacity <= 0x10000);
    return (ek_pool*) malloc(ek_pool_dynamic_size(capacity));
}

void ek_pool_init(ek_pool* pool, uint32_t capacity) {
    EK_ASSERT(pool);
    uint32_t* ids = pool->ids;
    // reserve invalid 0 id
    pool->next = 1;
    pool->size = 1;
    *ids = 0;
    ++ids;
#pragma nounroll
    for (int i = 2; i < capacity; ++i) {
        // to fit all indices we need to clamp capacity to (0xFFFF - 1)
        // next   : 0 2 ... 0xFFFF |      0
        // indexes: 0 1 ... 0xFFFE | 0xFFFF
        *ids = i;
        ++ids;
    }
    // set last free index to 0 (out of handles)
    *ids = 0;
}

uint32_t ek_pool_make_id(ek_pool* pool) {
    EK_ASSERT(pool);

    uint32_t* ids = pool->ids;
    const uint32_t next = pool->next;
    const uint32_t next_id = ids[next];
    pool->next = next_id & MASK_IDX;
    const uint32_t id = (next_id & MASK_GEN) | next;
    ids[next] = id;
    ++pool->size;
    return id;
}

bool ek_pool_valid_id(ek_pool* pool, uint32_t id) {
    return id != 0 && id == pool->ids[id & MASK_IDX];
}

void ek_pool_destroy_id(ek_pool* pool, uint32_t id) {
    EK_ASSERT(pool);
    EK_ASSERT(ek_pool_valid_id(pool, id));
    uint32_t* ids = pool->ids;
    const uint32_t index = id & MASK_IDX;
    const uint32_t version = (id + ADD_GEN) & MASK_GEN;
    ids[index] = version | pool->next;
    pool->next = index;
    --pool->size;
}

void ek_image_reg_setup(void) {
    memset(&ek_image_reg_, 0, sizeof(ek_image_reg));
    ek_pool_init((ek_pool*)&ek_image_reg_, 128);
    // reserve 0 slot as invalid or default
    ek_image_reg_.count = 1;
}

// return existing data index by name
// if not found creates new data handle, associate it with name, reset data to default state from data[0]
ek_image_reg_id ek_image_reg_named(const char* name) {
    for (uint16_t i = 1; i < ek_image_reg_.count; ++i) {
        if(strcmp(ek_image_reg_.names[i].str, name) == 0) {
            const uint16_t id_index = ek_image_reg_.dense_to_sparse[i];
            return (ek_image_reg_id){ek_image_reg_.ids[id_index]};
        }
    }
    uint32_t new_id = ek_pool_make_id((ek_pool*)&ek_image_reg_);
    uint16_t new_idx = new_id & MASK_IDX;
    EK_ASSERT(new_id != 0);
    uint16_t data_idx = ek_image_reg_.count;
    ++ek_image_reg_.count;
    ek_image_reg_.sparse_to_dense[new_idx] = data_idx;
    ek_image_reg_.dense_to_sparse[data_idx] = new_idx;
    strcpy(ek_image_reg_.names[data_idx].str, name);
    memcpy(ek_image_reg_.data + data_idx, &ek_image_reg_.data[0], sizeof(ek_image_reg_.data[0]));
    return (ek_image_reg_id){new_id};
}

void ek_image_reg_assign(ek_image_reg_id id, sg_image image) {
    EK_ASSERT(id.id != 0);
    EK_ASSERT(ek_pool_valid_id((ek_pool*)&ek_image_reg_, id.id));
    const uint16_t data_idx = ek_image_reg_.sparse_to_dense[id.id & MASK_IDX];
    sg_image* p = ek_image_reg_.data + data_idx;
    if(p->id) {
        sg_destroy_image(*p);
    }
    *p = image;
}

sg_image ek_image_reg_get(ek_image_reg_id id) {
    EK_ASSERT(id.id != 0);
    EK_ASSERT(ek_pool_valid_id((ek_pool*)&ek_image_reg_, id.id));
    const uint16_t data_idx = ek_image_reg_.sparse_to_dense[id.id & MASK_IDX];
    return ek_image_reg_.data[data_idx];
}
