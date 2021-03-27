#pragma once

/**
 * fill uninitialized memory with 0xCC symbol for not optimized build, whenever it could be useful for debugging,
 * but recommended to turn on `-Os` option for Debug build profile to make sure all uninitialized memory issues
 */
#ifndef NDEBUG
#define EK_INIT_CC_MEMORY
#endif

#ifndef NDEBUG
#define EK_ASSERTION_PEDANTIC
#endif

/**
 * Allocation tracker find simple issues on memory allocation / de-allocation, provide way to monitor arena status
 *
 * If one allocator used by multiple threads - code should be disabled
 *
 */
#ifndef NDEBUG
#define EK_ALLOCATION_TRACKER
#define EK_ALLOCATION_TRACKER_STATS
#endif