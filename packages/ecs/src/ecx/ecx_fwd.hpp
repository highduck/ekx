#pragma once

#include "ecx.h"
#include <stddef.h>

namespace ecs {

template<typename ...Component>
class ViewForward;

template<typename ...Component>
class ViewBackward;

struct Entity final : public entity_t {

    inline constexpr Entity() noexcept: entity_t{0} {}

    inline constexpr Entity(std::nullptr_t) noexcept: entity_t{0} {}

    inline constexpr Entity(entity_t c_entity) noexcept: entity_t{c_entity} {}

    inline constexpr Entity(Entity&& e) noexcept = default;

    inline constexpr Entity(const Entity& e) noexcept = default;

    inline constexpr Entity& operator=(Entity&& e) noexcept = default;

    inline constexpr Entity& operator=(const Entity& e) noexcept = default;

    inline constexpr Entity& operator=(std::nullptr_t) noexcept {
        id = 0u;
        return *this;
    }

    inline bool operator==(Entity other) const {
        return id == other.id;
    }

    inline bool operator!=(Entity other) const {
        return id != other.id;
    }

    inline bool operator==(std::nullptr_t) const {
        return id == 0u;
    }

    inline bool operator!=(std::nullptr_t) const {
        return id != 0u;
    }

    inline explicit operator bool() const {
        return id != 0u;
    }

    // TODO: remove, change to single or `is_entity`
    [[nodiscard]] inline bool valid() const;

    /** components manipulations **/

    template<typename Component>
    inline Component& add() const;

    template<typename C1, typename C2, typename ...Components>
    inline void add() const;

    template<typename Component>
    [[nodiscard]] inline bool has() const;

    template<typename Component>
    inline Component& get() const;

    template<typename Component>
    inline Component* try_get() const;

    template<typename Component>
    inline const Component& get_or_default() const;

    template<typename Component>
    bool remove() const;

    operator entity_id_t () const = delete;
    operator int32_t () const = delete;
    //operator uint32_t () const = delete;
    operator int64_t () const = delete;
    operator uint64_t () const = delete;
};

//#pragma pack()

}
