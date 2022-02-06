#pragma once

#include "ecx.h"
#include <stddef.h>

namespace ecs {

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

    inline constexpr EntityApi(entity_t entity_index) noexcept: index{entity_index} {}

    EntityApi(entity_passport_value_t passport_value) = delete;
    //EntityApi(uint32_t v) = delete;
    EntityApi(int32_t v) = delete;
    EntityApi(uint64_t v) = delete;
    EntityApi(int64_t v) = delete;
    EntityApi(uintptr_t v) = delete;
    EntityApi(intptr_t v) = delete;
    EntityApi(void* v) = delete;
    EntityApi(char* v) = delete;

    inline constexpr EntityApi(entity_passport_t passport) noexcept: index{resolve_entity_index(passport)} {}

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

    [[nodiscard]] inline bool is_alive() const;

    /** components manipulations **/

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

    inline constexpr operator entity_t() const noexcept {
        return index;
    }

    operator entity_passport_value_t () const = delete;
    operator int32_t () const = delete;
    //operator uint32_t () const = delete;
    operator int64_t () const = delete;
    operator uint64_t () const = delete;
};

struct EntityRef final {
    inline constexpr EntityRef() noexcept: passport{0u} {
    }

    inline constexpr EntityRef(std::nullptr_t) noexcept: passport{0} {
    }

//    inline constexpr explicit EntityRef(entity_t entity) noexcept:EntityRef{get_entity_passport(entity)}  {
//    }

    inline constexpr explicit EntityRef(entity_passport_t passport_) noexcept: passport{passport_} {
    }

    inline explicit EntityRef(EntityApi ent) noexcept;

    inline constexpr EntityRef(entity_t entity, entity_gen_t generation) noexcept:
            passport{.index=entity, .gen=generation} {
    }

    inline constexpr EntityRef(EntityRef&& e) noexcept = default;

    inline constexpr EntityRef(const EntityRef& e) noexcept = default;

    inline constexpr EntityRef& operator=(EntityRef&& e) noexcept = default;

    inline constexpr EntityRef& operator=(const EntityRef& e) noexcept = default;

    inline constexpr EntityRef& operator=(std::nullptr_t) noexcept {
        passport.value = 0;
        return *this;
    }

    // TODO: rename version to Generation,
    // TODO: rename index to entity_index,
    [[nodiscard]]
    inline constexpr entity_gen_t version() const noexcept {
        return passport.gen;
    }

    inline constexpr void version(entity_gen_t generation) noexcept {
        passport.gen = generation;
    }

    [[nodiscard]] inline constexpr entity_t index() const noexcept {
        return passport.index;
    }

    [[nodiscard]] inline constexpr EntityApi ent() const noexcept {
        return EntityApi{index()};
    }

    inline constexpr void index(entity_t entity) noexcept {
        passport.index = entity;
    }

    inline bool operator==(EntityRef other) const {
        return passport.value == other.passport.value;
    }

    inline bool operator!=(EntityRef other) const {
        return passport.value != other.passport.value;
    }

    inline bool operator==(std::nullptr_t) const {
        return passport.value == 0u;
    }

    inline bool operator!=(std::nullptr_t) const {
        return passport.value != 0u;
    }

    inline explicit operator bool() const {
        return passport.value != 0u;
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
