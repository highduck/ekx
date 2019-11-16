#pragma once

#include <cstddef>
#include "spec.hpp"

namespace ecxx {

class entity final {
public:
    using value_type = spec::value_type;
    using index_type = spec::index_type;
    using version_type = spec::version_type;

    inline constexpr entity() noexcept
            : value_{spec::null_value} {

    }

    inline constexpr explicit entity(value_type v) noexcept
            : value_{v} {

    }

    inline constexpr entity(index_type i, version_type v) noexcept
            : value_{i | (v << spec::index_bits)} {

    }

    inline constexpr entity(entity&& e) noexcept = default;

    inline constexpr entity(const entity& e) noexcept = default;

    inline constexpr entity& operator=(entity&& e) noexcept = default;

    inline constexpr entity& operator=(const entity& e) noexcept = default;

    [[nodiscard]] inline constexpr version_type version() const noexcept {
        return (value_ >> spec::index_bits) & spec::version_mask;
    }

    inline constexpr void version(version_type v) noexcept {
        value_ = (value_ & spec::index_mask) | (v << spec::index_bits);
    }

    [[nodiscard]] inline constexpr index_type index() const noexcept {
        return value_ & spec::index_mask;
    }

    inline constexpr void index(index_type v) noexcept {
        value_ = (value_ & ~spec::index_mask) | v;
    }

    inline bool operator==(entity other) const {
        return value_ == other.value_;
    }

    inline bool operator!=(entity other) const {
        return value_ != other.value_;
    }

    inline bool operator==(std::nullptr_t) const {
        return value_ == spec::null_value;
    }

    inline bool operator!=(std::nullptr_t) const {
        return value_ != spec::null_value;
    }

    inline explicit operator bool() const {
        return value_ != spec::null_value;
    }

private:
    value_type value_;
};

}