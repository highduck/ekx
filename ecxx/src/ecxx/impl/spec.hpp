#pragma once

#include <cstdint>

namespace ecxx::spec {

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

/**
 * Type for Component Type Identifies (usually continuously increased index)
 */
using component_typeid = uint32_t;

/**
 * Value used for NULL entity identification
 */
constexpr value_type null_value = 0u;

/**
 * Bits count for Entity Index encoding
 */
constexpr uint32_t index_bits = 20u;

/**
 * Bit mask to clamp Entity's Index
 */
constexpr uint32_t index_mask = (1u << index_bits) - 1u;

/**
 * Bits count for Entity's Version encoding
 */
constexpr uint32_t version_bits = 12u;

/**
 * Bit mask to clamp Entity's Version
 */
constexpr uint32_t version_mask = (1u << version_bits) - 1u;

}