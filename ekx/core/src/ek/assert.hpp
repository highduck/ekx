#pragma once

#include "config.hpp"

// EK_ASSERTION_PEDANTIC
// - out-of-bounds, allocation errors and others cases
// - disabled even for DEBUG build by default

// EK_ASSERTION_DEFAULT
// - like default assets(), disabled for NDEBUG builds

#if !defined(EK_ASSERTION_DEFAULT) && !defined(NDEBUG)
#define EK_ASSERTION_DEFAULT
#endif

#if defined(EK_ASSERTION_PEDANTIC) || defined(EK_ASSERTION_DEFAULT)

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>

inline void ek_assertion_handler(const char* e, const char* file, int line) {
    EM_ASM({console.assert(false, "%s from %s:%d", UTF8ToString($0), UTF8ToString($1), $2)}, e, file, line);
}

#else

#include <csignal>
#define EK_ABORT raise(SIGABRT)

#include <cstdio>
#define EK_LOG_EXCEPTION printf

#define ek_assertion_handler(e, file, line) ((void)EK_LOG_EXCEPTION("%s:%d: failed assertion '%s'\n", file, line, e), (void)EK_ABORT)
#endif

#define ek_assert(e) (!(e) ? ek_assertion_handler(#e, __FILE__, __LINE__) : ((void)0))

#endif

#ifdef EK_ASSERTION_DEFAULT
#define EK_ASSERT(x) ek_assert(x)
#endif

#ifdef EK_ASSERTION_PEDANTIC
#define EK_ASSERT_R2(x) ek_assert(x)
#endif

#ifndef EK_ASSERT
#define EK_ASSERT(x) ((void)0)
#endif

#ifndef EK_ASSERT_R2
#define EK_ASSERT_R2(x) ((void)0)
#endif