#ifndef EK_TEMP_RES_MAN_H
#define EK_TEMP_RES_MAN_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sokol_gfx.h>
#include <ek/assert.h>

/**
 * virtual resource reference
 */

// make stable identifiers to data slots (handles)

#ifdef __cplusplus
extern "C" {
#endif

#define MASK_IDX 0xFFFF
#define MASK_GEN 0xFFFF0000
#define ADD_GEN 0x00010000

typedef struct ek_pool {
    // allocated count
    uint32_t size;
    // next free index
    uint32_t next;

    uint32_t ids[];
} ek_pool;

#define ek_pool_dynamic_size(cap) (2 * sizeof(uint32_t) + (cap) * sizeof(uint32_t))

ek_pool* ek_pool_alloc(uint32_t capacity);

void ek_pool_init(ek_pool* pool, uint32_t capacity);

uint32_t ek_pool_make_id(ek_pool* pool);

bool ek_pool_valid_id(ek_pool* pool, uint32_t id);

void ek_pool_destroy_id(ek_pool* pool, uint32_t id);

typedef struct ek_texture_reg_id {
    uint32_t id;
} ek_texture_reg_id;

typedef struct ek_texture_reg_name {
    char str[64];
} ek_texture_reg_name;

void ek_texture_reg_setup();

// return existing data index by name
// if not found creates new data handle, associate it with name, reset data to default state from data[0]
ek_texture_reg_id ek_texture_reg_named(const char* name);

void ek_texture_reg_assign(ek_texture_reg_id id, sg_image image);

sg_image ek_texture_reg_get(ek_texture_reg_id id);

#ifdef __cplusplus
}
#endif

#endif // EK_TEMP_RES_MAN_H
