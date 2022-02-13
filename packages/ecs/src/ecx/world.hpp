#pragma once

#include <ek/assert.h>
#include <ek/sparse_array.h>
#include "ecx_fwd.hpp"

// for `std::is_empty`
#include <type_traits>
// for generic templated comp, auto ctor/dtor binding
#include <new>

namespace ecs {

/** Templated generic **/

template<typename T, unsigned Mode = 1>
class GenericComponent {
public:
    inline static ecx_component_type type;

    static void setup(uint16_t capacity, const char* label) {
        const ecx_component_type_decl decl{
                label,
                capacity,
                std::is_empty_v<T> ? 0 : 1,
                {sizeof(T)}
        };
        init_component_type(&type, decl);
        if constexpr((Mode & 1) != 0) type.ctor = construct;
        if constexpr((Mode & 2) != 0) type.dtor = destruct;
    }

    static void construct(component_handle_t handle) {
        T* ptr = ((T*) type.data[0]) + handle;
        new(ptr)T();
    }

    static void destruct(component_handle_t handle) {
        T* ptr = ((T*) type.data[0]) + handle;
        ptr->~T();
    }
};

template<typename T>
class ComponentType final : public GenericComponent<T> {
};

template<typename C>
[[nodiscard]]
constexpr ecx_component_type* type() noexcept {
    return &ComponentType<C>::type;
}

template<typename C>
inline C& add(entity_t e) {
    return *(C*) add_component(type<C>(), e);
}

template<typename C1, typename C2, typename ...Cn>
inline void add(entity_t e) {
    add<C1>(e);
    add<C2>(e);
    (add<Cn>(e), ...);
}

template<typename C>
inline bool has(entity_t e) {
    return get_component_handle(type<C>(), e) != 0;
}

template<typename C>
inline C& get(entity_t e) {
    return *(C*) get_component(type<C>(), e);
}

template<typename C>
inline C* try_get(entity_t e) {
    return (C*) get_component(type<C>(), e);
}

template<typename C>
inline bool remove(entity_t e) {
    return remove_component(type<C>(), e);
}

template<typename C>
inline const C& get_or_default(entity_t e) {
    return *(C*) get_component_or_default(type<C>(), e);
}

template<typename ...Cn>
inline ecs::Entity create() {
    entity_t e = create_entity();
    add<Cn...>(e);
    return Entity{e};
}

template<typename C>
inline bool has_type() {
    // if component is registered, non-zero type index will be assigned (component_type_id)
    return type<C>()->index != 0;
}

}