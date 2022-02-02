#pragma once

#include <ek/ds/Array.hpp>
#include <ek/ds/PodArray.hpp>
#include <ek/assert.h>
#include <ek/sparse_array.h>
#include "../ecxx_fwd.hpp"

// for `std::is_empty`
#include <type_traits>

namespace ecs {

/** Component Managers **/

class World;

// 256 + 74 = 330 bytes
// 128 * 330 = 42'240 kb
struct ComponentHeader {
    ek::PodArray<entity_t> handleToEntity; // dynamic (8 + 4 + 4 ~ 16 bytes)
    ek_sparse_array entityToHandle; // 256 byte

    // Debug info
    const char* debug_name = nullptr;
    uint16_t debug_data_stride = 0;
    uint16_t debug_type_index = 0;
    uint16_t debug_lock_counter = 0;

//    [[nodiscard]] inline unsigned count() const {
//        return handleToEntity.size();
//    }

    static int compareBySize(const void* a, const void* b) {
        return (int) (*(const ComponentHeader**) a)->handleToEntity.size() -
               (int) (*(const ComponentHeader**) b)->handleToEntity.size();
    }
};

/**
* Entity Pool generates dense indices for Entity representation.
* We track Generations on reusing deleted entities.
* Generation is 8-bit, so I swear to check references next update.
* Collision is possible if in one frame we do destroy and create more than 256 times
*/

/** World **/
class World {
public:
    // entity pool data, indices and generations
    // SIZE: 200 kb

    // entity indices
    entity_t indices[ECX_ENTITIES_MAX_COUNT]; // 2 * ENTITIES_MAX_COUNT = 131072 bytes

    // entity generations
    entity_gen_t generations[ECX_ENTITIES_MAX_COUNT]; // 65536 bytes

    //// control section
    // allocated entities count
    uint32_t size;

    // next free index in entity pool
    entity_t next;

    // just zero entity (reserved null entity)
    entity_t zero;

    // per component, data manager
    ComponentHeader* components[ECX_COMPONENTS_MAX_COUNT]; // 8 * 128 = 1024 bytes
    ecs_erase_callback_t callbacks_erase[ECX_COMPONENTS_MAX_COUNT];
    ecs_shutdown_callback_t callbacks_shutdown[ECX_COMPONENTS_MAX_COUNT];

    uint32_t components_num;
    uint32_t callbacks_erase_num;
    uint32_t callbacks_shutdown_num;

    /** entity pool **/

    template<typename ...Component>
    entity_t create();

    /** components **/

    template<typename Component>
    [[nodiscard]] inline bool has(entity_t e) const;

    template<typename Component>
    [[nodiscard]] inline Component& get(entity_t e) const;

    template<typename Component>
    [[nodiscard]] inline Component* tryGet(entity_t e) const;

//    template<typename Component, typename ...Args>
//    inline Component& assign(entity_t e, Args&& ... args) const;
//
//    template<typename Component, typename ...Args>
//    inline Component& reassign(entity_t e, Args&& ... args) const;

    template<typename Component>
    inline Component& assign(entity_t e) const;

    template<typename Component>
    inline Component& reassign(entity_t e) const;

    template<typename Component>
    inline void remove(entity_t e) const;

    template<typename Component>
    inline bool tryRemove(entity_t e) const;

    template<typename Component>
    inline Component& getOrCreate(entity_t e) const;

    template<typename Component>
    inline const Component& getOrDefault(entity_t e) const;

    template<typename Component>
    inline void registerComponent(unsigned initialCapacity = 4);

    template<typename Component>
    inline bool hasComponent();
};

/** Templated generic **/
/////////////// xpr: static version

template<typename T, typename DataArray>
class GenericComponent {
public:
    inline static ComponentHeader header{};
    inline static DataArray data{};

    static void setup(uint32_t capacity) {
        EK_ASSERT(data.empty());
        data.reserve(capacity);
        data.emplace_back();
        header.entityToHandle = ek_sparse_array_create(ECX_ENTITIES_MAX_COUNT);
        header.handleToEntity.reserve(capacity);
        header.handleToEntity.push_back(0);
#ifndef NDEBUG
        header.debug_name = ek::TypeName<T>::value;
        header.debug_data_stride = sizeof(T);
#endif
    }

    static void shutdown() {
        // drop all data, call destructors
        data.reset();
    }

    static T* emplace(entity_t entity) {
        auto entityToHandle = header.entityToHandle;
        auto& handleToEntity = header.handleToEntity;
        const component_handle_t handle = header.handleToEntity.size();
        EK_ASSERT(header.debug_lock_counter == 0);
        EK_ASSERT(ek_sparse_array_get(entityToHandle, entity) == 0);

        ek_sparse_array_insert(entityToHandle, entity, handle);
        handleToEntity.push_back(entity);
        if constexpr (std::is_empty_v<T>) {
            return data.begin();
        } else {
            return &data.emplace_back();
        }
    }

    static void try_erase(entity_t entity) {
        auto handle = ek_sparse_array_get(header.entityToHandle, entity);
        if (handle != 0) {
            erase(entity);
        }
    }

    static void erase(entity_t entity) {
        EK_ASSERT(header.debug_lock_counter == 0);
        EK_ASSERT(ek_sparse_array_get(header.entityToHandle, entity) != 0);
        const entity_t backEntity = header.handleToEntity.back();
        if (entity != backEntity) {
            const component_handle_t handle = ek_sparse_array_move_remove(header.entityToHandle, entity, backEntity);
            header.handleToEntity[handle] = backEntity;
            header.handleToEntity.pop_back();
            if constexpr (!std::is_empty_v<T>) {
                data.swap_remove(handle);
            }
        } else {
            ek_sparse_array_set(header.entityToHandle, entity, 0);
            header.handleToEntity.pop_back();
            if constexpr (!std::is_empty_v<T>) {
                data.pop_back();
            }
        }
    }

    static T* get_by_entity(entity_t e) {
        if constexpr (std::is_empty_v<T>) {
            return data.begin();
        } else {
            return get_by_handle(ek_sparse_array_get(header.entityToHandle, e));
        }
    }

    static T* get_by_handle(component_handle_t handle) {
        if constexpr (std::is_empty_v<T>) {
            return data.begin();
        } else {
            return data.begin() + handle;
        }
    }
};

template<typename T>
class SparseComponent {
public:
    inline static ComponentHeader header{};

    static void setup(uint32_t capacity) {
        header.entityToHandle = ek_sparse_array_create(ECX_ENTITIES_MAX_COUNT);
        header.handleToEntity.reserve(capacity);
        header.handleToEntity.push_back(0);
#ifndef NDEBUG
//        header.debug_name = ek::TypeName<T>::value;
//        header.debug_data_stride = sizeof(T);
#endif
    }

    static void shutdown() {}

    static T* emplace(entity_t entity) {
        auto entityToHandle = header.entityToHandle;
        auto& handleToEntity = header.handleToEntity;
        const component_handle_t handle = header.handleToEntity.size();
        EK_ASSERT(header.debug_lock_counter == 0);
        EK_ASSERT(ek_sparse_array_get(entityToHandle, entity) == 0);

        ek_sparse_array_insert(entityToHandle, entity, handle);
        handleToEntity.push_back(entity);

        return nullptr;
    }

    static void try_erase(entity_t entity) {
        auto handle = ek_sparse_array_get(header.entityToHandle, entity);
        if (handle != 0) {
            erase(entity);
        }
    }

    static void erase(entity_t entity) {
        EK_ASSERT(header.debug_lock_counter == 0);
        EK_ASSERT(ek_sparse_array_get(header.entityToHandle, entity) != 0);
        const entity_t backEntity = header.handleToEntity.back();
        if (entity != backEntity) {
            const component_handle_t handle = ek_sparse_array_move_remove(header.entityToHandle, entity, backEntity);
            header.handleToEntity[handle] = backEntity;
            header.handleToEntity.pop_back();
        } else {
            ek_sparse_array_set(header.entityToHandle, entity, 0);
            header.handleToEntity.pop_back();
        }
    }

    static T* get_by_entity(entity_t e) {
        (void)e;
        return nullptr;
    }

    static T* get_by_handle(component_handle_t handle) {
        (void)handle;
        return nullptr;
    }
};

template<typename T>
class C final : public GenericComponent<T, ek::PodArray<T>> {};

template<typename Component>
inline Component& World::assign(entity_t e) const {
    return *C<Component>::emplace(e);
}

template<typename Component>
inline Component& World::reassign(entity_t e) const {
    const auto handle = ek_sparse_array_get(C<Component>::header.entityToHandle, e);
    if (handle != 0) {
        Component* data = C<Component>::get_by_handle(handle);
        *data = {};
        return *data;
    }
    return *C<Component>::emplace(e);
}

template<typename Component>
inline bool World::has(entity_t e) const {
    return ek_sparse_array_get(C<Component>::header.entityToHandle, e) != 0;
}

template<typename Component>
inline Component& World::get(entity_t e) const {
    return *C<Component>::get_by_entity(e);
}

template<typename Component>
inline Component* World::tryGet(entity_t e) const {
    const auto handle = ek_sparse_array_get(C<Component>::header.entityToHandle, e);
    if (handle != 0) {
        return C<Component>::get_by_handle(handle);
    }
    return nullptr;
}

template<typename Component>
inline void World::remove(entity_t e) const {
    C<Component>().erase(e);
}

template<typename Component>
inline bool World::tryRemove(entity_t e) const {
    const auto handle = ek_sparse_array_get(C<Component>::header.entityToHandle, e);
    if (handle != 0) {
        C<Component>::erase(e);
        return true;
    }
    return false;
}

template<typename Component>
inline Component& World::getOrCreate(entity_t e) const {
    const auto handle = ek_sparse_array_get(C<Component>::header.entityToHandle, e);
    if (handle != 0) {
        return *C<Component>::get_by_handle(handle);
    }
    return *C<Component>::emplace(e);
}


template<typename Component>
inline const Component& World::getOrDefault(entity_t e) const {
    const auto handle = ek_sparse_array_get(C<Component>::header.entityToHandle, e);
    return *C<Component>::get_by_handle(handle);
}

template<typename ...Component>
inline entity_t World::create() {
    entity_t entity = ecx_create();
    (assign<Component>(entity), ...);
    return entity;
}

template<typename Component>
inline void World::registerComponent(unsigned initialCapacity) {
    C<Component>::setup(initialCapacity);
    add_ecs_erase_callback(C<Component>::try_erase);
    add_ecs_shutdown_callback(C<Component>::shutdown);

    // add component header
    EK_ASSERT(components_num < ECX_COMPONENTS_MAX_COUNT);
    const uint32_t index = components_num++;
    EK_ASSERT(components[index] == nullptr);
    components[index] = &C<Component>::header;
#ifndef NDEBUG
    C<Component>::header.debug_type_index = index;
#endif
}

template<typename Component>
inline bool World::hasComponent() {
    // if we have at least 1-length array - we are initialized
    return !C<Component>::header.handleToEntity.empty();
}

}