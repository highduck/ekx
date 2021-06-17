#pragma once

#include "impl/world.hpp"
#include "impl/view_forward.hpp"
#include "impl/view_backward.hpp"

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Component>
inline auto view() {
    return ViewForward<Component...>{the_world};
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto view_backward() {
    return ViewBackward<Component...>{the_world};
}

template<typename ...Component>
inline EntityApi create() {
    EntityIndex e;
    the_world.create<Component...>(&e, 1);
    return EntityApi{e};
}

[[nodiscard]] inline bool valid(EntityRef ref) {
    return ref && the_world.check(ref.passport);
}

inline void destroy(EntityApi e) {
    EK_ASSERT_R2(e.isAlive());
    the_world.destroy(&e.index, 1);
}

template<typename Func>
inline void each(Func func) {
    const auto* entities = the_world.entityPool;
    const auto count = the_world.size;

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
    return index && the_world.entityPool[index] == index;
}

template<typename Component, typename ...Args>
inline Component& EntityApi::assign(Args&& ... args) {
    EK_ASSERT_R2(isAlive());
    return the_world.assign<Component>(index, args...);
}

template<typename Component, typename ...Args>
inline Component& EntityApi::reassign(Args&& ... args) {
    EK_ASSERT_R2(isAlive());
    return the_world.reassign<Component>(index, args...);
}

template<typename Component>
[[nodiscard]] inline bool EntityApi::has() const {
    EK_ASSERT_R2(isAlive());
    return the_world.has<Component>(index);
}

template<typename Component>
inline Component& EntityApi::get() const {
    EK_ASSERT_R2(isAlive());
    return the_world.get<Component>(index);
}

template<typename Component>
inline Component* EntityApi::tryGet() const {
    EK_ASSERT_R2(isAlive());
    return the_world.tryGet<Component>(index);
}

template<typename Component>
inline Component& EntityApi::get_or_create() const {
    EK_ASSERT_R2(isAlive());
    return the_world.getOrCreate<Component>(index);
}

template<typename Component>
inline const Component& EntityApi::get_or_default() const {
    EK_ASSERT_R2(isAlive());
    return the_world.getOrDefault<Component>(index);
}

template<typename Component>
inline void EntityApi::remove() {
    EK_ASSERT_R2(isAlive());
    return the_world.remove<Component>(index);
}

template<typename Component>
inline bool EntityApi::try_remove() {
    EK_ASSERT_R2(isAlive());
    return the_world.tryRemove<Component>(index);
}

/** Entity methods impl **/

inline EntityRef::EntityRef(EntityApi ent) noexcept:
        EntityRef{ent.index, the_world.generation(ent.index)} {

}

inline bool EntityRef::valid() const {
    return passport && the_world.generation(index()) == version();
}

[[nodiscard]]
inline bool EntityRef::check(ecs::EntityApi e) const {
    return valid() && e.index == index();
}

[[nodiscard]]
inline EntityApi EntityRef::get() const {
    EK_ASSERT(the_world.check(passport));
    return EntityApi{index()};
}

inline bool EntityRef::invalidate() {
    if (passport) {
        if (the_world.generation(index()) == version()) {
            return true;
        } else {
            passport = 0;
        }
    }
    return false;
}

// compat

template<typename ...Component>
using view_forward_t = ViewForward<Component...>;

template<typename ...Component>
using view_backward_t = ViewBackward<Component...>;

}

#ifndef ECX_COMPONENT

#define ECX_COMPONENT(T) ecs::the_world.registerComponent<T>()
#define ECX_COMPONENT_RESERVE(T, cap) ecs::the_world.registerComponent<T>(cap)

#endif // #ifndef ECX_COMPONENT