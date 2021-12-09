#ifndef EK_CONFIG_H
#define EK_CONFIG_H

//#define EK_CONFIG_PARANOIA

/// Log trace and debug levels even for Release build
#ifdef EK_CONFIG_PARANOIA
#define EK_CONFIG_PROFILING
#define EK_CONFIG_LOG_ALL
#define EK_CONFIG_ASSERT_ALL
#endif

#ifndef NDEBUG
#define EK_DEBUG_BUILD
#endif // !NDEBUG

#if defined(EK_DEBUG_BUILD) || defined(EK_CONFIG_LOG_ALL)
#define EK_LOG_DEBUG_OUTPUT
#define EK_SOURCE_LOCATION_ENABLED
#endif

#ifdef EK_CONFIG_ASSERT_ALL
#define EK_ASSERTION_PEDANTIC
#endif

// Q: how to check uninitialized memory:
// A: turn on `-Os` option for Debug build profile to make sure all uninitialized memory issues

#endif // EK_CONFIG_H
