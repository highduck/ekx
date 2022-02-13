#pragma once

#include "ecx_fwd.hpp"
#include "world.hpp"
#include "view_forward.hpp"
#include "view_backward.hpp"

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Cn>
inline auto view() {
    return ViewForward<Cn...>();
}

template<typename C>
inline void foreach(void(* callback)(component_handle_t handle)) {
    foreach_type(type<C>(), callback);
}

template<typename C>
inline entity_t get_entity(component_handle_t handle) {
    return get_entity(type<C>(), handle);
}

template<typename C, unsigned Index = 0, typename DataType = C>
inline DataType* get_data(component_handle_t handle) {
    return (DataType*) type<C>()->data[Index] + handle;
}

template<typename C>
inline component_handle_t get_handle(entity_t entity) {
    return get_component_handle(type<C>(), entity);
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Cn>
inline auto view_backward() {
    return ViewBackward<Cn...>();
}

/** Entity methods impl **/

inline bool Entity::valid() const {
    return is_entity(*this);
}

template<typename C>
inline C& Entity::add() const {
    return ecs::add<C>(*this);
}

template<typename C1, typename C2, typename ...Cn>
inline void Entity::add() const {
    ecs::add<C1, C2, Cn...>(*this);
}

template<typename C>
[[nodiscard]] inline bool Entity::has() const {
    return ecs::has<C>(*this);
}

template<typename C>
inline C& Entity::get() const {
    return ecs::get<C>(*this);
}

template<typename C>
inline C* Entity::try_get() const {
    return ecs::try_get<C>(*this);
}

template<typename C>
inline const C& Entity::get_or_default() const {
    return ecs::get_or_default<C>(*this);
}

template<typename C>
inline bool Entity::remove() const {
    return ecs::remove<C>(*this);
}

}

#ifndef ECX_COMPONENT

#define ECX_COMPONENT(C) ecs::ComponentType<C>::setup(4, #C)
#define ECX_COMPONENT_RESERVE(C, cap) ecs::ComponentType<C>::setup((cap), #C)

#define ECX_COMP_TYPE_CXX(C) template<> class ecs::ComponentType<C> final : public ecs::GenericComponent<C, 3> {};

#endif // !ECX_COMPONENT