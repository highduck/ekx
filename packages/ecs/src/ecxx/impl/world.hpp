#pragma once

#include <ek/ds/Array.hpp>
#include <ek/ds/PodArray.hpp>
#include <ek/assert.h>
#include <ek/sparse_array.h>
#include "../ecxx_fwd.hpp"

// for `std::is_empty`
#include <type_traits>
// for generic templated comp, auto ctor/dtor binding
#include <new>

namespace ecs {

/** entity pool **/

template<typename ...Component>
ecs::EntityApi create();

/** components **/
//
//template<typename Component>
//[[nodiscard]] inline bool has(entity_t e);
//
//template<typename Component>
//[[nodiscard]] inline Component& get(entity_t e);
//
//template<typename Component>
//[[nodiscard]] inline Component* tryGet(entity_t e);
//
//template<typename Component>
//inline Component& assign(entity_t e);
//
//template<typename Component>
//inline Component& reassign(entity_t e);
//
//template<typename Component>
//inline void remove(entity_t e);
//
//template<typename Component>
//inline bool tryRemove(entity_t e);
//
//template<typename Component>
//inline Component& getOrCreate(entity_t e);
//
//template<typename Component>
//inline const Component& getOrDefault(entity_t e);
//
//template<typename Component>
//inline bool hasComponent();

/** Templated generic **/
/////////////// xpr: static version

template<typename T, unsigned Mode = 1>
class GenericComponent {
public:
    // declare as sparse component
    static_assert(!std::is_empty_v<T>);

    inline static ecx_component_type* header;

    static void setup(uint16_t capacity, const char* label) {
        header = ecx_new_component({label, (uint16_t) capacity, 1, {sizeof(T)}});
        if constexpr((Mode & 1) != 0) header->ctor = construct;
        if constexpr((Mode & 2) != 0) header->dtor = destruct;
    }

    static void construct(component_handle_t handle) {
        T* ptr = ((T*) header->data[0]) + handle;
        new(ptr)T();
    }

    static void destruct(component_handle_t handle) {
        T* ptr = ((T*) header->data[0]) + handle;
        ptr->~T();
    }

    static T* emplace(entity_t entity) {
        const component_handle_t handle = ecx_component_emplace(header, entity);
        return get_by_handle(handle);
    }

    static T* get_by_entity(entity_t e) {
        return get_by_handle(ek_sparse_array_get(header->entityToHandle, e));
    }

    static T* get_by_handle(component_handle_t handle) {
        return (T*) header->data[0] + handle;
    }
};

template<typename T>
class SparseComponent {
public:
    inline static ecx_component_type* header;

    static void setup(uint32_t capacity, const char* label) {
        header = ecx_new_component({label, (uint16_t) capacity, 0, {0}});
    }

    static T* emplace(entity_t entity) {
        ecx_component_emplace(header, entity);
        return nullptr;
    }

    static T* get_by_entity(entity_t e) {
        (void) e;
        return nullptr;
    }

    static T* get_by_handle(component_handle_t handle) {
        (void) handle;
        return nullptr;
    }
};

template<typename T>
class C final : public GenericComponent<T> {
};

template<typename Component>
inline Component& assign(entity_t e) {
    const component_handle_t handle = ecx_component_emplace(C<Component>::header, e);
    return *C<Component>::get_by_handle(handle);
}

template<typename Component>
inline Component& reassign(entity_t e) {
    const auto handle = ek_sparse_array_get(C<Component>::header->entityToHandle, e);
    if (handle != 0) {
        Component* data = C<Component>::get_by_handle(handle);
        *data = {};
        return *data;
    }
    return assign<Component>(e);
}

template<typename Component>
inline bool has(entity_t e) {
    return ek_sparse_array_get(C<Component>::header->entityToHandle, e) != 0;
}

template<typename Component>
inline Component& get(entity_t e) {
    return *C<Component>::get_by_entity(e);
}

template<typename Component>
inline Component* tryGet(entity_t e) {
    const auto handle = ek_sparse_array_get(C<Component>::header->entityToHandle, e);
    if (handle != 0) {
        return C<Component>::get_by_handle(handle);
    }
    return nullptr;
}

template<typename Component>
inline void remove(entity_t e) {
    ecx_component_erase(C<Component>::header, e);
}

template<typename Component>
inline bool try_remove(entity_t e) {
    const auto handle = ek_sparse_array_get(C<Component>::header->entityToHandle, e);
    if (handle != 0) {
        remove<Component>(e);
        return true;
    }
    return false;
}

template<typename Component>
inline Component& get_or_create(entity_t e) {
    const auto handle = ek_sparse_array_get(C<Component>::header->entityToHandle, e);
    if (handle != 0) {
        return *C<Component>::get_by_handle(handle);
    }
    return *C<Component>::emplace(e);
}


template<typename Component>
inline const Component& get_or_default(entity_t e) {
    const auto handle = ek_sparse_array_get(C<Component>::header->entityToHandle, e);
    return *C<Component>::get_by_handle(handle);
}

template<typename ...Component>
inline ecs::EntityApi create() {
    entity_t entity = ecx_create();
    (assign<Component>(entity), ...);
    return EntityApi{entity};
}

template<typename Component>
inline bool hasComponent() {
    // if we have at least 1-length array - we are initialized
    return C<Component>::header != 0;
}

}