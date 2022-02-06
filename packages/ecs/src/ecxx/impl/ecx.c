#include <ecxx/ecx.h>
#include <ek/buf.h>
#include <ek/sparse_array.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

ecx_world_t ecx;

entity_t ecx_indices[ECX_ENTITIES_MAX_COUNT];

entity_gen_t ecx_generations[ECX_ENTITIES_MAX_COUNT];

ecx_component_type ecx_components[ECX_COMPONENTS_MAX_COUNT];

static void ecx_reset_entity_pool() {
    // reset pool ids
    entity_t* it = ecx_indices;
#pragma nounroll
    for (uint32_t i = 1; i <= ECX_ENTITIES_MAX_COUNT; ++i) {
        *(it++) = i;
    }

    // reserve null entity
    ecx_indices[0] = 0;
    // is invalid
    ecx_generations[0] = 1;

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
    for (uint32_t i = 0; i < ecx.components_num; ++i) {
        ecx_component_type* type = &ecx_components[i];
        if (type->dtor) {
#pragma nounroll
            for (component_handle_t j = 1; j < type->size; ++j) {
                type->dtor(j);
            }
        }
        for (uint32_t j = 0; j < type->data_num; ++j) {
            ek_buf_reset(&type->data[j]);
        }
        if (type->on_shutdown) {
            type->on_shutdown();
        }
        ecx_components[i].size = 0;
    }
    ecx.components_num = 0;
}

entity_passport_t get_entity_passport(entity_t entity) {
    entity_passport_t passport;
    passport.index = entity;
    passport.gen = ecx_generations[entity];
    return passport;
}

entity_t resolve_entity_index(entity_passport_t passport) {
    const entity_t index = passport.index;
    const entity_gen_t i = ecx_generations[index];
    return i == passport.gen ? index : 0;
}

entity_passport_t entity_passport_val(entity_passport_value_t val) {
    return (entity_passport_t){.value = val};
}

bool check_entity_passport(entity_passport_t passport) {
    return ecx_generations[passport.index] == passport.gen;
}

bool check_entity_alive(entity_t entity) {
    return entity && ecx_indices[entity] == entity;
}

ecx_component_type* ecx_new_component(ecx_component_type_decl decl) {
    EK_ASSERT(ecx.components_num < ECX_COMPONENTS_MAX_COUNT);
    uint16_t index = ecx.components_num++;
    ecx_component_type* comp = &ecx_components[index];
    comp->index = index;
    comp->label = decl.label;

    comp->entityToHandle = ek_sparse_array_create(ECX_ENTITIES_MAX_COUNT);
    ek_buf_set_size((void**) &comp->handleToEntity, sizeof(entity_t), 1, decl.capacity);
    comp->size = 1;

    comp->data_num = decl.data_num;
    EK_ASSERT(decl.data_num < ECX_COMPONENT_DATA_LAYERS_MAX_COUNT);

#pragma nounroll
    for (uint32_t i = 0; i < decl.data_num; ++i) {
        comp->data_stride[i] = decl.data_stride[i];
        ek_buf_set_size((void**) &comp->data[i], decl.data_stride[i], 1, decl.capacity);
    }

    return comp;
}

// entity create / destroy

entity_t ecx_create(void) {
    EK_ASSERT(ecx.size < ECX_ENTITIES_MAX_COUNT);
    const entity_t next = ecx.next;
    ecx.next = ecx_indices[next];
    ecx_indices[next] = next;
    ++ecx.size;
    return next;
}

void ecx_destroy(entity_t entity) {
    EK_ASSERT_R2(check_entity_alive(entity));

    // destroy components
#pragma nounroll
    for (uint32_t i = 0; i < ecx.components_num; ++i) {
        component_handle_t handle = ek_sparse_array_get(ecx_components[i].entityToHandle, entity);
        if (handle != 0) {
            ecx_component_erase(&ecx_components[i], entity);
        }
        if (ecx_components[i].on_entity_destroy) {
            ecx_components[i].on_entity_destroy(entity);
        }
    }

    // destroy from POOL
    EK_ASSERT(ecx.size > 1);
    ecx_indices[entity] = ecx.next;
    ++ecx_generations[entity];
    ecx.next = entity;
    --ecx.size;
}

int ecx_component_type_compare(const void* a, const void* b) {
    const uint16_t size1 = (*(const ecx_component_type**) a)->size;
    const uint16_t size2 = (*(const ecx_component_type**) b)->size;
    return (int) size1 - (int) size2;
}

component_handle_t ecx_component_emplace(ecx_component_type* header, entity_t entity) {
    EK_ASSERT(header->lock_counter == 0);

    ek_sparse_array entityToHandle = header->entityToHandle;
    const component_handle_t handle = header->size++;
    EK_ASSERT(ek_sparse_array_get(entityToHandle, entity) == 0);

    ek_sparse_array_insert(entityToHandle, entity, handle);

    arr_maybe_grow((void**) &header->handleToEntity, sizeof(entity_t));
    ek_buf_header(header->handleToEntity)->length++;
    header->handleToEntity[handle] = entity;

    for (uint32_t i = 0; i < header->data_num; ++i) {
        void** p_arr = &header->data[0];
        uint16_t stride = header->data_stride[0];
        arr_maybe_grow(p_arr, stride);
        arr_add_(p_arr, stride);
    }

    if (header->ctor) {
        header->ctor(handle);
    }
    return handle;
}

component_handle_t ecx_component_erase(ecx_component_type* header, entity_t entity) {
    EK_ASSERT(header->lock_counter == 0);
    EK_ASSERT(ek_sparse_array_get(header->entityToHandle, entity) != 0);
    EK_ASSERT(header->size > 1);
    const component_handle_t last = --header->size;
    const entity_t back_entity = header->handleToEntity[last];
    arr_pop(header->handleToEntity);
    if (entity != back_entity) {
        const component_handle_t handle = ek_sparse_array_move_remove(header->entityToHandle, entity, back_entity);
        if (header->dtor) {
            header->dtor(handle);
        }
        for (uint32_t i = 0; i < header->data_num; ++i) {
            void* arr = header->data[i];
            uint16_t stride = header->data_stride[i];
            // [removed_handle] <-- [last]
            memcpy((char*) arr + handle * stride, (char*) arr + last * stride, stride);
            arr_pop(arr);
        }
        header->handleToEntity[handle] = back_entity;
        return handle;
    } else {
        ek_sparse_array_set(header->entityToHandle, entity, 0);
        if (header->dtor) {
            header->dtor(last);
        }
        for (uint32_t i = 0; i < header->data_num; ++i) {
            arr_pop(header->data[i]);
        }
        return last;
    }
}

#ifdef __cplusplus
}
#endif
