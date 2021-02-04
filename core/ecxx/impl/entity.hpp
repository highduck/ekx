#pragma once

#include <cstddef>
#include <cstdint>
#include "world.hpp"

namespace ecs {

#pragma pack(1)
class entity final {
public:

public:

    inline constexpr entity() noexcept: index{0} {}

    inline constexpr entity(std::nullptr_t) noexcept: index{0} {}

    inline constexpr explicit entity(Entity entity_index) noexcept: index{entity_index} {}

    inline constexpr entity(entity&& e) noexcept = default;

    inline constexpr entity(const entity& e) noexcept = default;

    inline constexpr entity& operator=(entity&& e) noexcept = default;

    inline constexpr entity& operator=(const entity& e) noexcept = default;

    inline constexpr entity& operator=(std::nullptr_t) noexcept {
        index = 0u;
        return *this;
    }

    inline bool operator==(entity other) const {
        return index == other.index;
    }

    inline bool operator!=(entity other) const {
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

    template<typename Component, typename ...Args>
    inline Component& assign(Args&& ... args);

    template<typename Component, typename ...Args>
    inline Component& reassign(Args&& ... args);

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

    Entity index;
};


class EntityRef final {
public:

    inline constexpr EntityRef() noexcept: passport{0u} {
    }

    inline constexpr EntityRef(std::nullptr_t) noexcept: passport{0} {
    }

    inline constexpr explicit EntityRef(Passport passport_) noexcept: passport{passport_} {
    }

    inline explicit EntityRef(entity ent) noexcept;

    inline constexpr EntityRef(Entity entity, Generation generation) noexcept:
            passport{static_cast<Passport>(entity) | (generation << INDEX_BITS)} {
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
    [[nodiscard]] inline constexpr Generation version() const noexcept {
        return (passport >> INDEX_BITS) & GENERATION_MASK;
    }

    inline constexpr void version(Generation generation) noexcept {
        passport = (passport & INDEX_MASK) | (generation << INDEX_BITS);
    }

    [[nodiscard]] inline constexpr Entity index() const noexcept {
        return passport & INDEX_MASK;
    }

    [[nodiscard]] inline constexpr entity ent() const noexcept {
        return entity{static_cast<Entity>(passport & INDEX_MASK)};
    }

    inline constexpr void index(Entity entity) noexcept {
        passport = (passport & ~INDEX_MASK) | entity;
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

    inline bool valid() const;

    [[nodiscard]]
    inline entity get() const {
        ECXX_ASSERT(the_world.check(passport));
        return entity{index()};
    }

    Passport passport;
};
#pragma pack()

}