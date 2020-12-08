#pragma once

#include <ecxx/impl/world.hpp>
#include <ecxx/impl/view_forward.hpp>
#include <ecxx/impl/view_backward.hpp>
#include <ecxx/impl/view_runtime.hpp>

namespace ecs {

/**
 * NULL entity value
 */

template<typename ...Component>
inline auto view() {
    return view_forward_t<Component...>{world::the};
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto view_backward() {
    return view_backward_t<Component...>{world::the};
}

template<typename It>
inline view_runtime_t runtime_view(It begin, It end) {
    std::vector<entity_map_base*> table;
    for (auto it = begin; it != end; ++it) {
        auto* set = world::the.components[*it];
        if (set != nullptr) {
            table.emplace_back(set);
        }
    }
    return view_runtime_t(table);
}

template<typename ...Component>
inline entity create() {
    return world::the.create<Component...>();
}

template<typename ...Component, typename It>
void create(It begin, It end) {
    return world::the.create<Component...>(begin, end);
}

[[nodiscard]] inline bool valid(entity e) {
    return e && world::the.entities.current(e.index()) == e.version();
}

inline void destroy(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.destroy(e);
}

template<typename Component>
[[nodiscard]] inline bool is_locked() {
    return world::the.is_locked<Component>();
}

template<typename Func>
inline void each(Func func) {
    return world::the.entities.each(func);
}

/** Entity methods functional style **/

template<typename Component, typename ...Args>
inline Component& assign(entity e, Args&& ... args) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.assign<Component>(e, args...);
}

template<typename Component, typename ...Args>
inline Component& reassign(entity e, Args&& ... args) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.reassign<Component>(e, args...);
}

template<typename Component>
[[nodiscard]] inline bool has(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.has<Component>(e.index());
}

template<typename Component>
inline Component& get(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.get<Component>(e.index());
}

template<typename Component>
inline Component& get_or_create(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.get_or_create<Component>(e);
}

template<typename Component>
inline const Component& get_or_default(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.get_or_default<Component>(e.index());
}

template<typename Component>
inline void remove(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.remove<Component>(e.index());
}

template<typename Component>
inline bool try_remove(entity e) {
    ECXX_FULL_ASSERT(valid(e));
    return world::the.try_remove<Component>(e.index());
}

/** Entity methods impl **/

inline bool entity::valid() const {
    return ecs::valid(*this);
}

template<typename Component, typename ...Args>
inline Component& entity::assign(Args&& ... args) {
    ECXX_FULL_ASSERT(valid());
    return world::the.assign<Component>(*this, args...);
}

template<typename Component, typename ...Args>
inline Component& entity::reassign(Args&& ... args) {
    ECXX_FULL_ASSERT(valid());
    return world::the.reassign<Component>(*this, args...);
}

template<typename Component>
[[nodiscard]] inline bool entity::has() const {
    ECXX_FULL_ASSERT(valid());
    return world::the.has<Component>(index());
}

template<typename Component>
inline Component& entity::get() const {
    ECXX_FULL_ASSERT(valid());
    return world::the.get<Component>(index());
}

template<typename Component>
inline Component* entity::tryGet() const {
    ECXX_FULL_ASSERT(valid());
    return world::the.tryGet<Component>(index());
}

template<typename Component>
inline Component& entity::get_or_create() const {
    ECXX_FULL_ASSERT(valid());
    return world::the.get_or_create<Component>(*this);
}

template<typename Component>
inline const Component& entity::get_or_default() const {
    ECXX_FULL_ASSERT(valid());
    return world::the.get_or_default<Component>(index());
}

template<typename Component>
inline void entity::remove() {
    ECXX_FULL_ASSERT(valid());
    return world::the.remove<Component>(index());
}

template<typename Component>
inline bool entity::try_remove() {
    ECXX_FULL_ASSERT(valid());
    return world::the.try_remove<Component>(index());
}

}