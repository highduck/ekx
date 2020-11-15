#pragma once

#include <cstddef>
#include <cstdint>

namespace ecs {

class entity final {
public:
/**
 * Type used for Entity Value code (entity version + entity index)
 */
    using value_type = uint32_t;

/**
 * Type used for Entity Index
 */
    using index_type = uint32_t;

/**
 * Type used for Entity version
 */
    using version_type = uint16_t;

private:
    // Bits count for Entity Index encoding
    static constexpr uint32_t index_bits = 20u;

    // Bit mask to clamp Entity's Index
    static constexpr uint32_t index_mask = (1u << index_bits) - 1u;

    // Bits count for Entity's Version encoding
    static constexpr uint32_t version_bits = 12u;

    // Bit mask to clamp Entity's Version
    static constexpr uint32_t version_mask = (1u << version_bits) - 1u;
public:

    inline constexpr entity() noexcept:
            value_{0u} {
    }

    inline constexpr entity(std::nullptr_t) noexcept:
            value_{0u} {
    }

    inline constexpr explicit entity(value_type v) noexcept:
            value_{v} {
    }

    inline constexpr entity(index_type i, version_type v) noexcept:
            value_{i | (v << index_bits)} {
    }

    inline constexpr entity(entity&& e) noexcept = default;

    inline constexpr entity(const entity& e) noexcept = default;

    inline constexpr entity& operator=(entity&& e) noexcept = default;

    inline constexpr entity& operator=(const entity& e) noexcept = default;

    inline constexpr entity& operator=(std::nullptr_t) noexcept {
        value_ = 0u;
        return *this;
    }

    [[nodiscard]] inline constexpr version_type version() const noexcept {
        return (value_ >> index_bits) & version_mask;
    }

    inline constexpr void version(version_type ver) noexcept {
        value_ = (value_ & index_mask) | (ver << index_bits);
    }

    [[nodiscard]] inline constexpr index_type index() const noexcept {
        return value_ & index_mask;
    }

    inline constexpr void index(index_type idx) noexcept {
        value_ = (value_ & ~index_mask) | idx;
    }

    inline bool operator==(entity other) const {
        return value_ == other.value_;
    }

    inline bool operator!=(entity other) const {
        return value_ != other.value_;
    }

    inline bool operator==(std::nullptr_t) const {
        return value_ == 0u;
    }

    inline bool operator!=(std::nullptr_t) const {
        return value_ != 0u;
    }

    inline explicit operator bool() const {
        return value_ != 0u;
    }

    [[nodiscard]] inline value_type passport() const {
        return value_;
    }

    inline bool valid() const;

    /** components manipulations **/

    template<typename Component, typename ...Args>
    inline Component& assign(Args&& ... args);

    template<typename Component, typename ...Args>
    inline Component& reassign(Args&& ... args);

    template<typename Component>
    [[nodiscard]] inline bool has() const;

    template<typename Component>
    inline Component& get();

    template<typename Component>
    inline Component& get_or_create();

    template<typename Component>
    inline const Component& get_or_default() const;

    template<typename Component>
    inline void remove();

    template<typename Component>
    inline bool try_remove();

private:
    value_type value_;
};

}