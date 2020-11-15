#pragma once

#include <ecxx/impl/world.hpp>

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Component>
inline auto view() {
    return basic_view<Component...>{world::the.components_data()};
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto rview() {
    return basic_rview<Component...>{
            world::the.components_data()
    };
}

template<typename ...Component>
inline entity create() {
    return world::the.create<Component...>();
}

template<typename ...Component, typename It>
void create(It begin, It end) {
    return world::the.create<Component...>(begin, end);
}

inline void destroy(entity e) {
    return world::the.destroy(e);
}

template<typename Func>
inline void each(Func func) {
    return world::the.entities.each(func);
}

template<typename Component>
[[nodiscard]] inline bool is_locked() {
    return world::the.is_locked<Component>();
}

[[nodiscard]] inline bool valid(entity e) {
    return e && world::the.entities.current(e.index()) == e.version();
}

/** Entity methods functional style **/

template<typename Component, typename ...Args>
inline Component& assign(entity e, Args&& ... args) {
    return world::the.assign<Component>(e, args...);
}

template<typename Component, typename ...Args>
inline Component& reassign(entity e, Args&& ... args) {
    return world::the.reassign<Component>(e, args...);
}

template<typename Component>
[[nodiscard]] inline bool has(entity e) {
    return world::the.has<Component>(e.index());
}

template<typename Component>
inline Component& get(entity e) {
    return world::the.get<Component>(e.index());
}

template<typename Component>
inline Component& get_or_create(entity e) {
    return world::the.get_or_create<Component>(e);
}

template<typename Component>
inline const Component& get_or_default(entity e) {
    return world::the.get_or_default<Component>(e.index());
}

template<typename Component>
inline void remove(entity e) {
    return world::the.remove<Component>(e.index());
}

template<typename Component>
inline bool try_remove(entity e) {
    return world::the.try_remove<Component>(e.index());
}

/** Entity methods impl **/


template<typename Component, typename ...Args>
inline Component& entity::assign(Args&& ... args) {
    return world::the.assign<Component>(*this, args...);
}

template<typename Component, typename ...Args>
inline Component& entity::reassign(Args&& ... args) {
    return world::the.reassign<Component>(*this, args...);
}

template<typename Component>
[[nodiscard]] inline bool entity::has() const {
    return world::the.has<Component>(index());
}

template<typename Component>
inline Component& entity::get() {
    return world::the.get<Component>(index());
}

template<typename Component>
inline Component& entity::get_or_create() {
    return world::the.get_or_create<Component>(*this);
}

template<typename Component>
inline const Component& entity::get_or_default() const {
    return world::the.get_or_default<Component>(index());
}

template<typename Component>
inline void entity::remove() {
    return world::the.remove<Component>(index());
}

template<typename Component>
inline bool entity::try_remove() {
    return world::the.try_remove<Component>(index());
}

}