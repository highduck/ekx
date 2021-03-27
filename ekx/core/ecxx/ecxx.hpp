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
    return view_forward_t<Component...>{&the_world};
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto view_backward() {
    return view_backward_t<Component...>{&the_world};
}

template<typename ...Component>
inline entity create() {
    Entity e;
    the_world.create<Component...>(&e, 1);
    return entity{e};
}

[[nodiscard]] inline bool valid(EntityRef ref) {
    return ref && the_world.check(ref.passport);
}

inline void destroy(entity e) {
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
            func(entity{index});
            ++processed;
        }
    }
}

/** Entity methods impl **/

inline bool entity::isAlive() const {
    return index && the_world.entityPool[index] == index;
}

template<typename Component, typename ...Args>
inline Component& entity::assign(Args&& ... args) {
    EK_ASSERT_R2(isAlive());
    return the_world.assign<Component>(index, args...);
}

template<typename Component, typename ...Args>
inline Component& entity::reassign(Args&& ... args) {
    EK_ASSERT_R2(isAlive());
    return the_world.reassign<Component>(index, args...);
}

template<typename Component>
[[nodiscard]] inline bool entity::has() const {
    EK_ASSERT_R2(isAlive());
    return the_world.has<Component>(index);
}

template<typename Component>
inline Component& entity::get() const {
    EK_ASSERT_R2(isAlive());
    return the_world.get<Component>(index);
}

template<typename Component>
inline Component* entity::tryGet() const {
    EK_ASSERT_R2(isAlive());
    return the_world.tryGet<Component>(index);
}

template<typename Component>
inline Component& entity::get_or_create() const {
    EK_ASSERT_R2(isAlive());
    return the_world.getOrCreate<Component>(index);
}

template<typename Component>
inline const Component& entity::get_or_default() const {
    EK_ASSERT_R2(isAlive());
    return the_world.getOrDefault<Component>(index);
}

template<typename Component>
inline void entity::remove() {
    EK_ASSERT_R2(isAlive());
    return the_world.remove<Component>(index);
}

template<typename Component>
inline bool entity::try_remove() {
    EK_ASSERT_R2(isAlive());
    return the_world.tryRemove<Component>(index);
}

/** Entity methods impl **/

inline EntityRef::EntityRef(entity ent) noexcept:
        EntityRef{ent.index, the_world.generation(ent.index)} {

}

inline bool EntityRef::valid() const {
    return passport && the_world.generation(index()) == version();
}

}