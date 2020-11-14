#pragma once

#include <ecxx/impl/context.hpp>

namespace ecs {

using entity = ecxx::entity;
using world = ecxx::world;

/**
 * NULL entity value
 */
constexpr entity null{};

template<typename ...Component>
inline auto view() {
    return ecxx::basic_view<Component...>{
            ecxx::get_world().components_data()
    };
}

/** special view provide back-to-front iteration
    and allow modify primary component map during iteration
 **/
template<typename ...Component>
inline auto rview() {
    return ecxx::basic_rview<Component...>{
            ecxx::get_world().components_data()
    };
}

template<typename ...Component>
inline entity create() {
    return ecxx::get_world().create<Component...>();
}

template<typename ...Component, typename It>
void create(It begin, It end) {
    return ecxx::get_world().create<Component...>(begin, end);
}

inline void destroy(entity e) {
    return ecxx::get_world().destroy(e);
}

template<typename Func>
inline void each(Func func) {
    return ecxx::get_world().each(func);
}

template<typename Component>
[[nodiscard]] inline bool is_locked() {
    return ecxx::get_world().is_locked<Component>();
}

[[nodiscard]] inline bool valid(entity e) {
    return ecxx::get_world().valid(e);
}


/** Entity methods functional style **/

template<typename Component, typename ...Args>
inline Component& assign(entity e, Args&& ... args) {
    return ecxx::get_world().assign<Component>(e, args...);
}

template<typename Component, typename ...Args>
inline Component& replace_or_assign(entity e, Args&& ... args) {
    return ecxx::get_world().replace_or_assign<Component>(e, args...);
}

template<typename Component>
[[nodiscard]] inline bool has(entity e) {
    return ecxx::get_world().has<Component>(e);
}

template<typename Component>
inline Component& get(entity e) {
    return ecxx::get_world().get<Component>(e);
}

template<typename Component>
inline Component& get_or_create(entity e) {
    return ecxx::get_world().get_or_create<Component>(e);
}

template<typename Component>
inline const Component& get_or_default(entity e) {
    return ecxx::get_world().get_or_default<Component>(e);
}

template<typename Component>
inline void remove(entity e) {
    return ecxx::get_world().remove<Component>(e);
}

template<typename Component>
inline bool try_remove(entity e) {
    return ecxx::get_world().try_remove<Component>(e);
}
}

namespace ecxx {

/** Entity methods impl **/


template<typename Component, typename ...Args>
inline Component& entity::assign(Args&& ... args) {
    return get_world().assign<Component>(*this, args...);
}

template<typename Component, typename ...Args>
inline Component& entity::replace_or_assign(Args&& ... args) {
    return get_world().replace_or_assign<Component>(*this, args...);
}

template<typename Component>
[[nodiscard]] inline bool entity::has() const {
    return get_world().has<Component>(*this);
}

template<typename Component>
inline Component& entity::get() {
    return get_world().get<Component>(*this);
}

template<typename Component>
inline Component& entity::get_or_create() {
    return get_world().get_or_create<Component>(*this);
}

template<typename Component>
inline const Component& entity::get_or_default() const {
    return get_world().get_or_default<Component>(*this);
}

template<typename Component>
inline void entity::remove() {
    return get_world().remove<Component>(*this);
}

template<typename Component>
inline bool entity::try_remove() {
    return get_world().try_remove<Component>(*this);
}

}