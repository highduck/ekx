#include "world.hpp"
#include <ek/log.h>
#include <ek/assert.h>
#include <cstring>

ecs::World ecx;

#ifdef __cplusplus
extern "C" {
#endif

static void ecx_reset_entity_pool() {
    // reset pool ids
    auto* it = ecx.indices;
#pragma nounroll
    for (uint32_t i = 1; i <= ECX_ENTITIES_MAX_COUNT; ++i) {
        *(it++) = i;
    }

    // reserve null entity
    ecx.indices[0] = 0;
    // is invalid
    ecx.generations[0] = 1;

    ecx.next = 1;
    ecx.size = 1;
}

// World create / destroy

void ecx_setup(void) {
    log_debug("ecs::world initialize");
    ecx_reset_entity_pool();
}

void ecx_shutdown(void) {
    log_debug("ecs::world shutdown");
#pragma nounroll
    for(uint32_t i = 0; i < ecx.callbacks_shutdown_num; ++i) {
        ecx.callbacks_shutdown[i]();
    }
    ecx.callbacks_shutdown_num = 0;
    ecx.callbacks_erase_num = 0;
}

entity_passport_t get_entity_passport(entity_t entity) {
    return entity | (ecx.generations[entity] << ECX_INDEX_BITS);
}

bool check_entity_passport(entity_passport_t passport) {
    const entity_gen_t i = ecx.generations[passport & ECX_INDEX_MASK];
    const entity_gen_t j = (passport >> ECX_INDEX_BITS) & ECX_GENERATION_MASK;
    return i == j;
}

bool check_entity_alive(entity_t entity) {
    return entity && ecx.indices[entity] == entity;
}

void add_ecs_erase_callback(ecs_erase_callback_t callback) {
    EK_ASSERT(ecx.callbacks_erase_num < ECX_COMPONENTS_MAX_COUNT);
    ecx.callbacks_erase[ecx.callbacks_erase_num++] = callback;
}

void add_ecs_shutdown_callback(ecs_shutdown_callback_t callback) {
    EK_ASSERT(ecx.callbacks_shutdown_num < ECX_COMPONENTS_MAX_COUNT);
    ecx.callbacks_shutdown[ecx.callbacks_shutdown_num++] = callback;
}

// entity create / destroy

entity_t ecx_create(void) {
    EK_ASSERT(ecx.size < ECX_ENTITIES_MAX_COUNT);
    const entity_t next = ecx.next;
    entity_t* index = ecx.indices + next;
    ecx.next = *index;
    *index = next;
    ++ecx.size;
    return next;
}

void ecx_destroy(entity_t entity) {
    EK_ASSERT_R2(check_entity_alive(entity));

    // destroy from POOL
    EK_ASSERT(ecx.size > 1);
    ecx.indices[entity] = ecx.next;
    ++ecx.generations[entity];
    ecx.next = entity;
    --ecx.size;

    // destroy components
#pragma nounroll
    for (uint32_t i = 0; i < ecx.callbacks_erase_num; ++i) {
        ecx.callbacks_erase[i](entity);
    }
}

#ifdef __cplusplus
}
#endif
