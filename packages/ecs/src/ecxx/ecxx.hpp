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

template<typename ...Component>
inline EntityApi create() {
    return EntityApi{ ecx.create<Component...>() };
}

template<typename Func>
inline void each(Func func) {
    const auto* entities = ecx.indices;
    const auto count = ecx.size;

    for (uint32_t i = 1, processed = 1; processed < count; ++i) {
        const auto index = entities[i];
        if (index == i) {
            func(EntityApi{index});
            ++processed;
        }
    }
}

/** Entity methods impl **/

inline bool EntityApi::isAlive() const {
    return index && ecx.indices[index] == index;
}

//template<typename Component, typename ...Args>
//inline Component& EntityApi::assign(Args&& ... args) {
//    EK_ASSERT_R2(isAlive());
//    return ecx.assign<Component>(index, args...);
//}
//
//template<typename Component, typename ...Args>
//inline Component& EntityApi::reassign(Args&& ... args) {
//    EK_ASSERT_R2(isAlive());
//    return ecx.reassign<Component>(index, args...);
//}

template<typename Component>
inline Component& EntityApi::assign() {
    EK_ASSERT_R2(isAlive());
    return ecx.assign<Component>(index);
}

template<typename Component>
inline Component& EntityApi::reassign() {
    EK_ASSERT_R2(isAlive());
    return ecx.reassign<Component>(index);
}

template<typename Component>
[[nodiscard]] inline bool EntityApi::has() const {
    EK_ASSERT_R2(isAlive());
    return ecx.has<Component>(index);
}

template<typename Component>
inline Component& EntityApi::get() const {
    EK_ASSERT_R2(isAlive());
    return ecx.get<Component>(index);
}

template<typename Component>
inline Component* EntityApi::tryGet() const {
    EK_ASSERT_R2(isAlive());
    return ecx.tryGet<Component>(index);
}

template<typename Component>
inline Component& EntityApi::get_or_create() const {
    EK_ASSERT_R2(isAlive());
    return ecx.getOrCreate<Component>(index);
}

template<typename Component>
inline const Component& EntityApi::get_or_default() const {
    EK_ASSERT_R2(isAlive());
    return ecx.getOrDefault<Component>(index);
}

template<typename Component>
inline void EntityApi::remove() {
    EK_ASSERT_R2(isAlive());
    return ecx.remove<Component>(index);
}

template<typename Component>
inline bool EntityApi::try_remove() {
    EK_ASSERT_R2(isAlive());
    return ecx.tryRemove<Component>(index);
}

/** Entity methods impl **/

inline EntityRef::EntityRef(EntityApi ent) noexcept:
        EntityRef{ent.index, ecx.generations[ent.index]} {

}

inline bool EntityRef::valid() const {
    return passport && ecx.generations[index()] == version();
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
    if (passport) {
        if (ecx.generations[index()] == version()) {
            return true;
        } else {
            passport = 0;
        }
    }
    return false;
}

}

#ifndef ECX_COMPONENT

#define ECX_COMPONENT(T) ecx.registerComponent<T>()
#define ECX_COMPONENT_RESERVE(T, cap) ecx.registerComponent<T>(cap)

#define ECX_COMP_TYPE_SPARSE(T) template<> class ecs::C<T> final : public ecs::SparseComponent<T> {};
#define ECX_COMP_TYPE_CXX(T) template<> class ecs::C<T> final : public ecs::GenericComponent<T, ek::Array<T>> {};

#endif // #ifndef ECX_COMPONENT