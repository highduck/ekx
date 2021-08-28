#pragma once

//#define EK_CONFIG_PARANOIA

/// Log trace and debug levels even for Release build
#ifdef EK_CONFIG_PARANOIA
#define EK_CONFIG_PROFILING
#define EK_CONFIG_LOG_ALL
#define EK_CONFIG_ASSERT_ALL
#define EK_CONFIG_MEMORY_DEBUG
#endif

#ifndef NDEBUG
#define EK_DEBUG_BUILD
#endif

#if defined(EK_DEBUG_BUILD) || defined(EK_CONFIG_LOG_ALL)
#define EK_LOG_DEBUG_OUTPUT
#define EK_SOURCE_LOCATION_ENABLED
#endif

#ifdef EK_CONFIG_ASSERT_ALL
#define EK_ASSERTION_PEDANTIC
#endif

#ifdef EK_CONFIG_MEMORY_DEBUG

/**
 * fill uninitialized memory with 0xCC symbol for not optimized build, whenever it could be useful for debugging,
 * but recommended to turn on `-Os` option for Debug build profile to make sure all uninitialized memory issues
 */
#define EK_INIT_CC_MEMORY

/**
 * Allocation tracker find simple issues on memory allocation / de-allocation, provide way to monitor arena status
 *
 * If one allocator used by multiple threads - code should be disabled
 *
 */
#ifndef EK_ALLOCATION_TRACKER
#define EK_ALLOCATION_TRACKER
#endif
#ifndef EK_ALLOCATION_TRACKER_STATS
#define EK_ALLOCATION_TRACKER_STATS
#endif

#endif
