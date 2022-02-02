#pragma once

#include "ecx.h"
#include <stddef.h>
#include <ek/util/Type.hpp>

namespace ecs {

class World;

template<typename ...Component>
class ViewForward;

template<typename ...Component>
class ViewBackward;

struct EntityApi;

struct EntityRef;

#pragma pack(1)

struct EntityApi final {

    inline constexpr EntityApi() noexcept: index{0} {}

    inline constexpr EntityApi(std::nullptr_t) noexcept: index{0} {}

    inline constexpr explicit EntityApi(entity_t entity_index) noexcept: index{entity_index} {}
    inline constexpr explicit EntityApi(entity_passport_t passport) noexcept: index{(entity_t)(passport & ECX_INDEX_MASK)} {}

    inline constexpr EntityApi(EntityApi&& e) noexcept = default;

    inline constexpr EntityApi(const EntityApi& e) noexcept = default;

    inline constexpr EntityApi& operator=(EntityApi&& e) noexcept = default;

    inline constexpr EntityApi& operator=(const EntityApi& e) noexcept = default;

    inline constexpr EntityApi& operator=(std::nullptr_t) noexcept {
        index = 0u;
        return *this;
    }

    inline bool operator==(EntityApi other) const {
        return index == other.index;
    }

    inline bool operator!=(EntityApi other) const {
        return index != other.index;
    }

    inline bool operator==(std::nullptr_t) const {
        return index == 0u;
    }

    inline bool operator!=(std::nullptr_t) const {
        return index != 0u;
    }

    inline explicit operator bool() const {
        return index != 0u;
    }

    inline bool isAlive() const;

    /** components manipulations **/

//    template<typename Component, typename ...Args>
//    inline Component& assign(Args&& ... args);
//
//    template<typename Component, typename ...Args>
//    inline Component& reassign(Args&& ... args);

    template<typename Component>
    inline Component& assign();

    template<typename Component>
    inline Component& reassign();

    template<typename Component>
    [[nodiscard]] inline bool has() const;

    template<typename Component>
    inline Component& get() const;

    template<typename Component>
    inline Component* tryGet() const;

    template<typename Component>
    inline Component& get_or_create() const;

    template<typename Component>
    inline const Component& get_or_default() const;

    template<typename Component>
    inline void remove();

    template<typename Component>
    inline bool try_remove();

    entity_t index;
};

struct EntityRef final {
    inline constexpr EntityRef() noexcept: passport{0u} {
    }

    inline constexpr EntityRef(std::nullptr_t) noexcept: passport{0} {
    }

    inline constexpr explicit EntityRef(entity_passport_t passport_) noexcept: passport{passport_} {
    }

    inline explicit EntityRef(EntityApi ent) noexcept;

    inline constexpr EntityRef(entity_t entity, entity_gen_t generation) noexcept:
            passport{static_cast<entity_passport_t>(entity) | (generation << ECX_INDEX_BITS)} {
    }

    inline constexpr EntityRef(EntityRef&& e) noexcept = default;

    inline constexpr EntityRef(const EntityRef& e) noexcept = default;

    inline constexpr EntityRef& operator=(EntityRef&& e) noexcept = default;

    inline constexpr EntityRef& operator=(const EntityRef& e) noexcept = default;

    inline constexpr EntityRef& operator=(std::nullptr_t) noexcept {
        passport = 0;
        return *this;
    }

    // TODO: rename version to Generation,
    // TODO: rename index to entity_index,
    [[nodiscard]]
    inline constexpr entity_gen_t version() const noexcept {
        return (passport >> ECX_INDEX_BITS) & ECX_GENERATION_MASK;
    }

    inline constexpr void version(entity_gen_t generation) noexcept {
        passport = (passport & ECX_INDEX_MASK) | (generation << ECX_INDEX_BITS);
    }

    [[nodiscard]] inline constexpr entity_t index() const noexcept {
        return passport & ECX_INDEX_MASK;
    }

    [[nodiscard]] inline constexpr EntityApi ent() const noexcept {
        return EntityApi{static_cast<entity_t>(passport & ECX_INDEX_MASK)};
    }

    inline constexpr void index(entity_t entity) noexcept {
        passport = (passport & ~ECX_INDEX_MASK) | entity;
    }

    inline bool operator==(EntityRef other) const {
        return passport == other.passport;
    }

    inline bool operator!=(EntityRef other) const {
        return passport != other.passport;
    }

    inline bool operator==(std::nullptr_t) const {
        return passport == 0u;
    }

    inline bool operator!=(std::nullptr_t) const {
        return passport != 0u;
    }

    inline explicit operator bool() const {
        return passport != 0u;
    }

    [[nodiscard]]
    inline bool valid() const;

    [[nodiscard]]
    inline bool check(ecs::EntityApi e) const;

    [[nodiscard]]
    inline EntityApi get() const;

    entity_passport_t passport;

    inline bool invalidate();
};

#pragma pack()

}

extern ecs::World ecx;
