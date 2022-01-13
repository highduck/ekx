#ifndef EK_ASSERT_H
#define EK_ASSERT_H

#include <ek/pre.h>

/** Assertions **/
// options for `ek_config.h`

// EK_ASSERTION_PEDANTIC
// - out-of-bounds, allocation errors and others cases
// - disabled even for DEBUG build by default

// EK_ASSERTION_DEFAULT
// - like default assets(), disabled for NDEBUG builds

#if !defined(EK_ASSERTION_DEFAULT) && !defined(NDEBUG)
#define EK_ASSERTION_DEFAULT
#endif

#if defined(EK_ASSERTION_PEDANTIC) || defined(EK_ASSERTION_DEFAULT)

#ifdef __cplusplus
extern "C" {
#endif

void ek_handle_assert(const char* e, const char* file, int line);

#ifdef __cplusplus
}
#endif

#define EK_ASSERT(e) (UNLIKELY(!(e)) ? ek_handle_assert(#e, EK_CURRENT_FILE, EK_CURRENT_LINE) : ((void)0))

#else

#define EK_ASSERT(e) ((void)(0))

#endif

#ifdef EK_ASSERTION_PEDANTIC
#define EK_ASSERT_R2(x) EK_ASSERT(x)
#else // EK_ASSERTION_PEDANTIC
#define EK_ASSERT_R2(x) ((void)0)
#endif // !EK_ASSERTION_PEDANTIC

#endif //EK_ASSERT_H
