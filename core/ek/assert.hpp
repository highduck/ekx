#pragma once

//#define EK_ASSERTION_PEDANTIC

// EK_ASSERTION_PEDANTIC
// - out-of-bounds, allocation errors and others cases
// - disabled even for DEBUG build by default

// EK_ASSERTION_DEFAULT
// - like default assets(), disabled for NDEBUG builds

#if defined(EK_ASSERTION_PEDANTIC) || defined(EK_ASSERTION_DEFAULT)

#include <cstdio>

#define ek_assertion_handler(e, file, line) ((void)printf("%s:%d: failed assertion `%s'\n", file, line, e), abort())
#define ek_assert(e) (!(e) ? ek_assertion_handler(#e, __FILE__, __LINE__) : ((void)0))

#endif

#ifdef EK_ASSERTION_DEFAULT
#define EK_ASSERT(x) ek_assert(x)
#else
#define EK_ASSERT(x) ((void)0)
#endif

#ifdef EK_ASSERTION_PEDANTIC
#define EK_ASSERT_R2(x) ek_assert(x)
#else
#define EK_ASSERT_R2(x) ((void)0)
#endif