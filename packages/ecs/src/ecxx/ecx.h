#ifndef ECX_H
#define ECX_H

#include <ek/sparse_array.h>

#ifdef __cplusplus

#include <cstdint>
#include <cstdbool>

extern "C" {
#else
#include <stdint.h>
#include <stdbool.h>
#endif


enum {
    ECX_COMPONENTS_MAX_COUNT = 128,
    ECX_COMPONENT_DATA_LAYERS_MAX_COUNT = 2,
    ECX_ENTITIES_MAX_COUNT = 0x10000,
    ECX_INDEX_BITS = 16,
    ECX_INDEX_MASK = 0xFFFF,
    ECX_GENERATION_MASK = 0xFF,
};

typedef uint16_t entity_t;
typedef uint8_t entity_gen_t;
typedef uint16_t component_handle_t;
typedef uint32_t entity_passport_value_t;

// Passport is compressed unique ID with information for: world index, entity index, entity generation
typedef union entity_passport_t {
    entity_passport_value_t value;
    struct {
        entity_t index;
        entity_gen_t gen;
    };
} entity_passport_t;

entity_t resolve_entity_index(entity_passport_t passport);
entity_passport_t entity_passport_val(entity_passport_value_t val);
bool check_entity_passport(entity_passport_t passport);
entity_passport_t get_entity_passport(entity_t entity);
bool check_entity_alive(entity_t entity);

/** lifecycle **/

void ecx_setup(void);

void ecx_shutdown(void);

// entities
entity_t ecx_create(void);

void ecx_destroy(entity_t entity);

typedef struct {
    uint8_t prev;
    uint8_t next;
} ecx_component_list_node;

typedef struct {
    const char* label;
    uint16_t capacity;
    uint16_t data_num;
    uint16_t data_stride[ECX_COMPONENT_DATA_LAYERS_MAX_COUNT];
} ecx_component_type_decl;

typedef struct {
    ek_sparse_array entityToHandle; // 0
    entity_t* handleToEntity; // 4
    // entities associated with this component, from 1 to MAX_ENTITIES,
    // 0 - is not initialized, because required at least zero-handle is allocated
    uint16_t size; // 8
    uint16_t data_stride[ECX_COMPONENT_DATA_LAYERS_MAX_COUNT]; // 10 .. 14
    uint16_t data_num; // 14

    void* data[ECX_COMPONENT_DATA_LAYERS_MAX_COUNT]; // 16 .. 24
    ecx_component_list_node* list; // 24
    void (* on_shutdown)(); // 28
    // if callback set, will be triggered when entity is destroyed (not matters  component is attached or not)
    void (* on_entity_destroy)(entity_t e); // 32
    void (* ctor)(component_handle_t handle);

    void (* dtor)(component_handle_t handle);

    // debug section
    const char* label; // 36
    /**
     * array index in global components(types) array obtained on registration,
     * used for linking enabled components for entity
     */
    uint16_t index; // 40
    uint16_t lock_counter; // 42
    uint16_t __pad[10]; // 44 .. 64
} ecx_component_type;

//static_assert(sizeof(ecx_component_header) == 64);

ecx_component_type* ecx_new_component(ecx_component_type_decl decl);

int ecx_component_type_compare(const void* a, const void* b);

component_handle_t ecx_component_emplace(ecx_component_type* header, entity_t entity);

component_handle_t ecx_component_erase(ecx_component_type* header, entity_t entity);

/**
* Entity Pool generates dense indices for Entity representation.
* We track Generations on reusing deleted entities.
* Generation is 8-bit, so I swear to check references next update.
* Collision is possible if in one frame we do destroy and create more than 256 times
*/

typedef struct ecx_world_t {
    // allocated entities count
    uint32_t size;

    // next free index in entity pool
    entity_t next;

    // just zero entity (reserved null entity)
    entity_t zero;

    uint32_t components_num;
} ecx_world_t;

// entity pool data, indices and generations
// entity indices
extern entity_t ecx_indices[ECX_ENTITIES_MAX_COUNT]; // 2 * ENTITIES_MAX_COUNT = 131072 bytes
// entity generations
extern entity_gen_t ecx_generations[ECX_ENTITIES_MAX_COUNT]; // 65536 bytes
// per component, data manager
extern ecx_component_type ecx_components[ECX_COMPONENTS_MAX_COUNT];

extern ecx_world_t ecx;

#ifdef __cplusplus
}
#endif

#endif // ECX_H
