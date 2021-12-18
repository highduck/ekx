#include <ek/assert.h>

#if defined(EK_DEBUG_BUILD) || defined(EK_ASSERTION_PEDANTIC)

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#else //__EMSCRIPTEN__

#include <stdio.h>
#include <signal.h>

#endif // !__EMSCRIPTEN__

//#include <stdnoreturn.h>

void ek_handle_assert(const char* e, const char* file, int line) {
#ifdef __EMSCRIPTEN__
    EM_ASM({console.assert(false, "%s from %s:%d", UTF8ToString($0), UTF8ToString($1), $2)}, e, file, line);
#else
    printf("%s:%d: failed assertion '%s'\n", file, line, e);
    raise(SIGABRT);
#endif
}

#endif
