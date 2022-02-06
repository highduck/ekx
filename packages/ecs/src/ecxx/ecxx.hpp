#pragma once

#include "ecxx_fwd.hpp"
#include "impl/world.hpp"
#include "impl/view_forward.hpp"
#include "impl/view_backward.hpp"

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Component>
inline auto view() {
    return ViewForward<Component...>();
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto view_backward() {
    return ViewBackward<Component...>();
}

//template<typename ...Component>
//inline EntityApi create() {
//    return EntityApi{create<Component...>()};
//}

template<typename Func>
inline void each(Func func) {
    const auto count = ecx.size;

    for (uint32_t i = 1, processed = 1; processed < count; ++i) {
        const auto index = ecx_indices[i];
        if (index == i) {
            func(EntityApi{index});
            ++processed;
        }
    }
}

/** Entity methods impl **/

inline bool EntityApi::is_alive() const {
    return index && ecx_indices[index] == index;
}

template<typename Component>
inline Component& EntityApi::assign() {
    EK_ASSERT(is_alive());
    return ecs::assign<Component>(index);
}

template<typename Component>
inline Component& EntityApi::reassign() {
    EK_ASSERT(is_alive());
    return ecs::reassign<Component>(index);
}

template<typename Component>
[[nodiscard]] inline bool EntityApi::has() const {
    EK_ASSERT(is_alive());
    return ecs::has<Component>(index);
}

template<typename Component>
inline Component& EntityApi::get() const {
    EK_ASSERT_R2(is_alive());
    return ecs::get<Component>(index);
}

template<typename Component>
inline Component* EntityApi::tryGet() const {
    EK_ASSERT(is_alive());
    return ecs::tryGet<Component>(index);
}

template<typename Component>
inline Component& EntityApi::get_or_create() const {
    EK_ASSERT(is_alive());
    return ecs::get_or_create<Component>(index);
}

template<typename Component>
inline const Component& EntityApi::get_or_default() const {
    EK_ASSERT(is_alive());
    return ecs::get_or_default<Component>(index);
}

template<typename Component>
inline void EntityApi::remove() {
    EK_ASSERT(is_alive());
    return ecs::remove<Component>(index);
}

template<typename Component>
inline bool EntityApi::try_remove() {
    EK_ASSERT(is_alive());
    return ecs::try_remove<Component>(index);
}

/** Entity methods impl **/

inline EntityRef::EntityRef(EntityApi ent) noexcept:
        EntityRef{ent.index, ecx_generations[ent.index]} {

}

inline bool EntityRef::valid() const {
    return check_entity_passport(passport);
}

[[nodiscard]]
inline bool EntityRef::check(ecs::EntityApi e) const {
    return valid() && e.index == index();
}

[[nodiscard]]
inline EntityApi EntityRef::get() const {
    EK_ASSERT(check_entity_passport(passport));
    return EntityApi{index()};
}

inline bool EntityRef::invalidate() {
    if (passport.value) {
        if (ecx_generations[index()] == version()) {
            return true;
        } else {
            passport.value = 0;
        }
    }
    return false;
}

}

#ifndef ECX_COMPONENT

#define ECX_COMPONENT(T) ecs::C<T>::setup(4, #T)
#define ECX_COMPONENT_RESERVE(T, cap) ecs::C<T>::setup((cap), #T)

#define ECX_COMP_TYPE_SPARSE(T) template<> class ecs::C<T> final : public ecs::SparseComponent<T> {};
#define ECX_COMP_TYPE_CXX(T) template<> class ecs::C<T> final : public ecs::GenericComponent<T, 3> {};

#endif // !ECX_COMPONENT