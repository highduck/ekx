#ifndef EK_TEMP_RES_MAN_H
#define EK_TEMP_RES_MAN_H

#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "gfx.h"
#include <ek/assert.h>

/**
 * virtual resource reference
 */

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_image_reg_id {
    uint32_t id;
} ek_image_reg_id;

typedef struct ek_shader_ref {
    uint32_t id;
} ek_shader_ref;

void ek_gfx_res_setup();

// return existing data index by name
// if not found creates new data handle, associate it with name, reset data to default state from data[0]
ek_image_reg_id ek_image_reg_named(const char* name);

void ek_image_reg_assign(ek_image_reg_id id, sg_image image);

sg_image ek_image_reg_get(ek_image_reg_id id);

ek_shader_ref ek_shader_named(const char* name);

void ek_shader_register(ek_shader_ref ref, ek_shader shader);

ek_shader ek_shader_get(ek_shader_ref ref);

#ifdef __cplusplus
}
#endif

#endif // EK_TEMP_RES_MAN_H
