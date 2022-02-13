#include "ecx.h"
#include <ek/buf.h>
#include <ek/sparse_array.h>
#include <ek/log.h>
#include <ek/assert.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

// forward
component_handle_t ecx__erase_data(ecx_component_type* type, entity_idx_t entity_idx);

// globals
ecx_world_t ecx;

entity_t ecx_pool[ECX_ENTITIES_MAX_COUNT];

component_type_id ecx_component_head[ECX_ENTITIES_MAX_COUNT];

ecx_component_type* ecx_components[ECX_COMPONENTS_MAX_COUNT];

on_entity_destroy ecx_on_entity_destroy[ECX_LISTENERS_MAX_COUNT];

static void ecx_reset_entity_pool() {
    // reset pool ids
    entity_t* it = ecx_pool;
#pragma nounroll
    for (uint32_t i = 1; i <= ECX_ENTITIES_MAX_COUNT; ++i) {
        (it++)->id = i;
    }

    // reserve null entity as invalid
    ecx_pool[0].id = -1;

    ecx.next = 1;
    ecx.size = 1;
}

// World create / destroy

void ecx_setup(void) {
    log_debug("ecs::world initialize");
    ecx_reset_entity_pool();
    ecx.components_num = 1;
}

void ecx_shutdown(void) {
    log_debug("ecs::world shutdown");
#pragma nounroll
    for (uint32_t i = 1; i < ecx.components_num; ++i) {
        ecx_component_type* type = ecx_components[i];
        if (type->dtor) {
#pragma nounroll
            for (component_handle_t j = 1; j < type->size; ++j) {
                type->dtor(j);
            }
        }
#pragma nounroll
        for (uint32_t j = 0; j < type->data_num; ++j) {
            ek_buf_reset(&type->data[j]);
        }
        if (type->on_shutdown) {
            type->on_shutdown();
        }
        type->size = 0;
    }
    ecx.components_num = 0;
}

entity_t entity_at(entity_idx_t idx) {
    return ecx_pool[idx];
}

//entity_t invalidate_entity(entity_t entity) {
//    return is_entity(entity) ? entity : NULL_ENTITY;
//}

entity_t entity_id(entity_id_t id) {
    return (entity_t) {
            .id = id
    };
}

bool is_entity(entity_t e) {
    return ecx_pool[e.idx].id == e.id;
}

void init_component_type(ecx_component_type* type, ecx_component_type_decl decl) {
    EK_ASSERT(ecx.components_num < ECX_COMPONENTS_MAX_COUNT);
    uint16_t index = ecx.components_num++;
    ecx_components[index] = type;
    type->index = index;
    type->label = decl.label;

    type->entityToHandle = ek_sparse_array_create(ECX_ENTITIES_MAX_COUNT);
    ek_buf_set_size((void**) &type->handleToEntity, sizeof(entity_t), 1, decl.capacity);
    ek_buf_set_size((void**) &type->component_next, sizeof(ecx_component_list_t), 1, decl.capacity);
    type->size = 1;

    type->data_num = decl.data_num;
    EK_ASSERT(decl.data_num < ECX_COMPONENT_DATA_LAYERS_MAX_COUNT);

#pragma nounroll
    for (uint32_t i = 0; i < decl.data_num; ++i) {
        type->data_stride[i] = decl.data_stride[i];
        ek_buf_set_size((void**) &type->data[i], decl.data_stride[i], 1, decl.capacity);
    }
}

// entity create / destroy

entity_t create_entity(void) {
    EK_ASSERT(ecx.size < ECX_ENTITIES_MAX_COUNT);
    const entity_idx_t next = (entity_idx_t) ecx.next;
    ecx.next = ecx_pool[next].idx;
    ecx_pool[next].idx = next;
    ++ecx.size;
    return ecx_pool[next];
}

void clear_entity(entity_t entity) {
    EK_ASSERT_R2(is_entity(entity));

    // destroy components
    component_type_id i = ecx_component_head[entity.idx];
#pragma nounroll
    while (i) {
        ecx_component_type* type = ecx_components[i];
        component_handle_t handle = get_component_handle_by_index(type, entity.idx);
        EK_ASSERT(handle != 0);
        i = type->component_next[handle];
        ecx__erase_data(type, entity.idx);
    }
    ecx_component_head[entity.idx] = 0;
}

void destroy_entity(entity_t entity) {
    clear_entity(entity);

    // notify on entity destroy
#pragma nounroll
    for (uint32_t i = 0; i < ecx.on_entity_destroy_num; ++i) {
        ecx_on_entity_destroy[i](entity);
    }

    // destroy entity and return free index to pool
    EK_ASSERT(ecx.size > 1);
    const entity_idx_t idx = entity.idx;
    ecx_pool[idx].idx = ecx.next;
    ++ecx_pool[idx].gen;
    ecx.next = idx;
    --ecx.size;
}

// internal function:
// - valid entity_index
// - component is not set
component_handle_t _create_component(ecx_component_type* type, entity_idx_t entity_idx) {
    EK_ASSERT(type && type->lock_counter == 0);
    EK_ASSERT_R2(get_component_handle_by_index(type, entity_idx) == 0);

    const component_handle_t handle = type->size++;
    ek_sparse_array_insert(type->entityToHandle, entity_idx, handle);

    arr_maybe_grow((void**) &type->handleToEntity, sizeof(entity_t));
    ek_buf_header(type->handleToEntity)->length++;
    type->handleToEntity[handle] = entity_idx;

    // link component
    arr_maybe_grow((void**) &type->component_next, sizeof(component_type_id));
    ek_buf_header(type->component_next)->length++;
    component_type_id next = ecx_component_head[entity_idx];
    ecx_component_head[entity_idx] = type->index;
    type->component_next[handle] = next;

    for (uint32_t i = 0; i < type->data_num; ++i) {
        void** p_arr = &type->data[0];
        uint16_t stride = type->data_stride[0];
        arr_maybe_grow(p_arr, stride);
        arr_add_(p_arr, stride);
    }

    if (type->ctor) {
        type->ctor(handle);
    }
    return handle;
}

void unlink_component(ecx_component_type* type, entity_idx_t entity_idx) {
    EK_ASSERT(type);
    component_type_id prev = 0;
    component_type_id i = ecx_component_head[entity_idx];
    while (i) {
        ecx_component_type* i_type = ecx_components[i];
        component_handle_t handle = get_component_handle_by_index(i_type, entity_idx);
        if (i_type == type) {
            if (!prev) {
                ecx_component_head[entity_idx] = i_type->component_next[handle];
            } else {
                ecx_components[prev]->component_next[get_component_handle_by_index(ecx_components[prev], entity_idx)]
                        = i_type->component_next[handle];
            };
            // maybe we could not zero
            i_type->component_next[handle] = 0;
            return;
        }
        prev = i;
        i = i_type->component_next[handle];
    }
    EK_ASSERT(0 && "component not found");
}

component_handle_t ecx__erase_data(ecx_component_type* type, entity_idx_t entity_idx) {
    EK_ASSERT(type->lock_counter == 0);
    EK_ASSERT(get_component_handle_by_index(type, entity_idx) != 0);
    EK_ASSERT(type->size > 1);
    component_handle_t last = --type->size;
    component_handle_t handle;
    const entity_idx_t back_entity_idx = type->handleToEntity[last];
    if (entity_idx != back_entity_idx) {
        handle = ek_sparse_array_move_remove(type->entityToHandle, entity_idx, back_entity_idx);
        if (type->dtor) {
            type->dtor(handle);
        }
        for (uint32_t i = 0; i < type->data_num; ++i) {
            void* arr = type->data[i];
            uint16_t stride = type->data_stride[i];
            // [removed_handle] <-- [last]
            memcpy((char*) arr + handle * stride, (char*) arr + last * stride, stride);
        }
        type->handleToEntity[handle] = back_entity_idx;
        type->component_next[handle] = type->component_next[last];
    } else {
        handle = last;
        ek_sparse_array_set(type->entityToHandle, entity_idx, 0);
        if (type->dtor) {
            type->dtor(handle);
        }
    }

    arr_pop(type->handleToEntity);
    arr_pop(type->component_next);
#pragma nounroll
    for (uint32_t i = 0; i < type->data_num; ++i) {
        arr_pop(type->data[i]);
    }

    return handle;
}

void foreach_entity(void(* callback)(entity_t)) {
    const uint32_t count = ecx.size;
    for (uint32_t idx = 1, processed = 1; processed < count; ++idx) {
        const entity_t e = ecx_pool[idx];
        if (e.idx == idx) {
            callback(e);
            ++processed;
        }
    }
}

void foreach_type(ecx_component_type* type, void(* callback)(component_handle_t)) {
    const uint16_t count = type->size;
    for (uint16_t i = 1; i < count; ++i) {
        callback((component_handle_t) i);
    }
}

component_handle_t get_component_handle_by_index(const ecx_component_type* type, entity_idx_t entity_idx) {
    EK_ASSERT(type);
    EK_ASSERT(type->entityToHandle.data && "component type is not initialized");
    return ek_sparse_array_get(type->entityToHandle, entity_idx);
}

component_handle_t get_component_handle(const ecx_component_type* type, entity_t entity) {
    EK_ASSERT(is_entity(entity));
    return get_component_handle_by_index(type, entity.idx);
}

entity_t get_entity(const ecx_component_type* type, component_handle_t handle) {
    EK_ASSERT(type);
    EK_ASSERT(handle);
    EK_ASSERT(handle < type->size);
    return entity_at(type->handleToEntity[handle]);
}

bool remove_component(ecx_component_type* type, entity_t entity) {
    EK_ASSERT(type && type->lock_counter == 0);
#ifndef NDEBUG
    if (UNLIKELY(!is_entity(entity))) {
        EK_ASSERT(0 && "entity is invalid");
        return false;
    }
#endif
    const component_handle_t handle = get_component_handle_by_index(type, entity.idx);
    if (LIKELY(handle)) {
        unlink_component(type, entity.idx);
        ecx__erase_data(type, entity.idx);
    }
    return handle;
}

void* get_component_data(ecx_component_type* type, component_handle_t handle, uint32_t data_index) {
    void* data = type->data[data_index];
    if (LIKELY(data)) {
        return data + handle * type->data_stride[data_index];
    }
    return (void*) (uintptr_t) handle;
}

void* get_component(ecx_component_type* type, entity_t entity) {
    EK_ASSERT(type);
#ifndef NDEBUG
    if (UNLIKELY(!is_entity(entity))) {
        EK_ASSERT(0 && "entity is invalid");
        return NULL;
    }
#endif
    const component_handle_t handle = get_component_handle_by_index(type, entity.idx);
    return handle ? get_component_data(type, handle, 0) : NULL;
}

void* get_component_or_default(ecx_component_type* type, entity_t entity) {
    EK_ASSERT(type);
#ifndef NDEBUG
    if (UNLIKELY(!is_entity(entity))) {
        EK_ASSERT(0 && "entity is invalid");
        return NULL;
    }
#endif
    const component_handle_t handle = get_component_handle_by_index(type, entity.idx);
    // we are not check handle with 0, because default data is recorded at 0 index, so we just fetch by index
    return get_component_data(type, handle, 0);
}

void* add_component(ecx_component_type* type, entity_t entity) {
    EK_ASSERT(type);
#ifndef NDEBUG
    if (UNLIKELY(!is_entity(entity))) {
        EK_ASSERT(0 && "entity is invalid");
        return NULL;
    }
#endif
    component_handle_t handle = get_component_handle_by_index(type, entity.idx);
    if (LIKELY(!handle)) {
        handle = _create_component(type, entity.idx);
    }
    return get_component_data(type, handle, 0);
}

static int compare_ecs_types_(const void* a, const void* b) {
    const uint16_t size1 = (*(const ecx_component_type**) a)->size;
    const uint16_t size2 = (*(const ecx_component_type**) b)->size;
    return (int) size1 - (int) size2;
}

void _sort_component_type_table(ecx_component_type** types_table, uint32_t count) {
    qsort(types_table, count, sizeof(ecx_component_type*), compare_ecs_types_);
}

#ifdef __cplusplus
}
#endif
