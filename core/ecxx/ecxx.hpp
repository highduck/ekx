#pragma once

#include <ecxx/impl/world.hpp>
#include <ecxx/impl/entity.hpp>
#include <ecxx/impl/view_forward.hpp>
#include <ecxx/impl/view_backward.hpp>

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Component>
inline auto view() {
    return view_forward_t<Component...>{the_world};
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto view_backward() {
    return view_backward_t<Component...>{the_world};
}

template<typename ...Component>
inline entity create() {
    Entity e;
    the_world->create<Component...>(&e, 1);
    return entity{e};
}

[[nodiscard]] inline bool valid(EntityRef ref) {
    return ref && the_world->check(ref.passport);
}

inline void destroy(entity e) {
    ECXX_FULL_ASSERT(e.isAlive());
    the_world->destroy(&e.index, 1);
}

template<typename Func>
inline void each(Func func) {
    const auto* entities = the_world->entities;
    const auto count = the_world->size;

    for (uint32_t i = 1, processed = 1; processed < count; ++i) {
        const auto index = entities[i];
        if (index == i) {
            func(entity{index});
            ++processed;
        }
    }
}

/** Entity methods impl **/

inline bool entity::isAlive() const {
    return index && the_world->entities[index] == index;
}

template<typename Component, typename ...Args>
inline Component& entity::assign(Args&& ... args) {
    ECXX_FULL_ASSERT(isAlive());
    return entity_assign<Component>(the_world, index, args...);
}

template<typename Component, typename ...Args>
inline Component& entity::reassign(Args&& ... args) {
    ECXX_FULL_ASSERT(isAlive());
    return entity_reassign<Component>(the_world, index, args...);
}

template<typename Component>
[[nodiscard]] inline bool entity::has() const {
    ECXX_FULL_ASSERT(isAlive());
    return the_world->has<Component>(index);
}

template<typename Component>
inline Component& entity::get() const {
    ECXX_FULL_ASSERT(isAlive());
    return the_world->get<Component>(index);
}

template<typename Component>
inline Component* entity::tryGet() const {
    ECXX_FULL_ASSERT(isAlive());
    return the_world->tryGet<Component>(index);
}

template<typename Component>
inline Component& entity::get_or_create() const {
    ECXX_FULL_ASSERT(isAlive());
    return entity_get_or_create<Component>(the_world, index);
}

template<typename Component>
inline const Component& entity::get_or_default() const {
    ECXX_FULL_ASSERT(isAlive());
    return entity_get_or_default<Component>(the_world, index);
}

template<typename Component>
inline void entity::remove() {
    ECXX_FULL_ASSERT(isAlive());
    return entity_remove<Component>(the_world, index);
}

template<typename Component>
inline bool entity::try_remove() {
    ECXX_FULL_ASSERT(isAlive());
    return entity_try_remove<Component>(the_world, index);
}

/** Entity methods impl **/

inline EntityRef::EntityRef(entity ent) noexcept:
        EntityRef{ent.index, the_world->generation(ent.index)} {

}

inline bool EntityRef::valid() const {
    return passport && the_world->generation(index()) == version();
}

template<typename Component, typename ...Args>
inline Component& EntityRef::assign(Args&& ... args) {
    ECXX_FULL_ASSERT(valid());
    return entity_assign<Component>(the_world, *this, args...);
}

template<typename Component, typename ...Args>
inline Component& EntityRef::reassign(Args&& ... args) {
    ECXX_FULL_ASSERT(valid());
    return entity_reassign<Component>(the_world, *this, args...);
}

template<typename Component>
[[nodiscard]] inline bool EntityRef::has() const {
    ECXX_FULL_ASSERT(valid());
    return the_world->has<Component>(index());
}

template<typename Component>
inline Component& EntityRef::get() const {
    ECXX_FULL_ASSERT(valid());
    return the_world->get<Component>(index());
}

template<typename Component>
inline Component* EntityRef::tryGet() const {
    ECXX_FULL_ASSERT(valid());
    return the_world->tryGet<Component>(index());
}

template<typename Component>
inline Component& EntityRef::get_or_create() const {
    ECXX_FULL_ASSERT(valid());
    return entity_get_or_create<Component>(the_world, *this);
}

template<typename Component>
inline const Component& EntityRef::get_or_default() const {
    ECXX_FULL_ASSERT(valid());
    return entity_get_or_default<Component>(the_world, index());
}

template<typename Component>
inline void EntityRef::remove() {
    ECXX_FULL_ASSERT(valid());
    return entity_remove<Component>(the_world, index());
}

template<typename Component>
inline bool EntityRef::try_remove() {
    ECXX_FULL_ASSERT(valid());
    return entity_try_remove<Component>(the_world, index());
}

}