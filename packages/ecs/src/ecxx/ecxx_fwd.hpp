#pragma once

#include <cstddef>
#include <cstdint>
#include <ek/util/Type.hpp>

namespace ecs {

class World;

extern World the_world;

template<typename ...Component>
class ViewForward;

template<typename ...Component>
class ViewBackward;

struct EntityApi;

struct EntityRef;

/** Static configuration definition **/
inline constexpr uint32_t COMPONENTS_MAX_COUNT = 128;
inline constexpr uint32_t ENTITIES_MAX_COUNT = 0x10000;
inline constexpr uint32_t INDEX_BITS = 16;
inline constexpr uint32_t INDEX_MASK = 0xFFFF;
inline constexpr uint32_t GENERATION_MASK = 0xFF;

typedef uint16_t EntityIndex;
typedef uint8_t EntityGeneration;
//typedef uint8_t WorldIndex;
typedef uint32_t EntityPassport; // Passport is compressed unique ID with information for: world index, entity index, entity generation

typedef uint16_t ComponentHandle;
typedef uint16_t ComponentTypeId;

#pragma pack(1)

struct EntityApi final {

    inline constexpr EntityApi() noexcept: index{0} {}

    inline constexpr EntityApi(std::nullptr_t) noexcept: index{0} {}

    inline constexpr explicit EntityApi(EntityIndex entity_index) noexcept: index{entity_index} {}

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

    EntityIndex index;
};

struct EntityRef final {
    inline constexpr EntityRef() noexcept: passport{0u} {
    }

    inline constexpr EntityRef(std::nullptr_t) noexcept: passport{0} {
    }

    inline constexpr explicit EntityRef(EntityPassport passport_) noexcept: passport{passport_} {
    }

    inline explicit EntityRef(EntityApi ent) noexcept;

    inline constexpr EntityRef(EntityIndex entity, EntityGeneration generation) noexcept:
            passport{static_cast<EntityPassport>(entity) | (generation << INDEX_BITS)} {
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
    inline constexpr EntityGeneration version() const noexcept {
        return (passport >> INDEX_BITS) & GENERATION_MASK;
    }

    inline constexpr void version(EntityGeneration generation) noexcept {
        passport = (passport & INDEX_MASK) | (generation << INDEX_BITS);
    }

    [[nodiscard]] inline constexpr EntityIndex index() const noexcept {
        return passport & INDEX_MASK;
    }

    [[nodiscard]] inline constexpr EntityApi ent() const noexcept {
        return EntityApi{static_cast<EntityIndex>(passport & INDEX_MASK)};
    }

    inline constexpr void index(EntityIndex entity) noexcept {
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

    [[nodiscard]]
    inline bool valid() const;

    [[nodiscard]]
    inline bool check(ecs::EntityApi e) const;

    [[nodiscard]]
    inline EntityApi get() const;

    EntityPassport passport;

    inline bool invalidate();
};

#pragma pack()

// compat

using world = World;

using entity = EntityApi;

//#define ECX_TYPE(Idx,Type) \
//template<> struct TypeIndex<Type,ecs::World>{constexpr static int value = (Idx);}; \
//template<> struct TypeName<Type>{constexpr static const char* value = #Type;};

#define ECX_TYPE(Idx,Type)

}