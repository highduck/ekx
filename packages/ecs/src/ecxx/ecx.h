#ifndef ECX_H
#define ECX_H

#ifdef __cplusplus

#include <cstdint>

extern "C" {
#else
#include <stdint.h>
#endif

enum {
    ECX_COMPONENTS_MAX_COUNT = 128,
    ECX_ENTITIES_MAX_COUNT = 0x10000,
    ECX_INDEX_BITS = 16,
    ECX_INDEX_MASK = 0xFFFF,
    ECX_GENERATION_MASK = 0xFF
};

typedef uint16_t entity_t;
typedef uint8_t entity_gen_t;
// Passport is compressed unique ID with information for: world index, entity index, entity generation
typedef uint32_t entity_passport_t;

typedef uint16_t component_handle_t;

bool check_entity_passport(entity_passport_t passport);
entity_passport_t get_entity_passport(entity_t entity);
bool check_entity_alive(entity_t entity);

typedef void(*ecs_erase_callback_t)(entity_t entity);
typedef void(*ecs_shutdown_callback_t)();

void add_ecs_erase_callback(ecs_erase_callback_t callback);
void add_ecs_shutdown_callback(ecs_shutdown_callback_t callback);

/** lifecycle **/

void ecx_setup(void);

void ecx_shutdown(void);

// entities
entity_t ecx_create(void);

void ecx_destroy(entity_t entity);

#ifdef __cplusplus
}
#endif

#endif // ECX_H
