#include "admob.hpp"
#include <ek/util/StaticStorage.hpp>
#include <ek/log.h>

// implementation
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if defined(__ANDROID__)

#include "admob_android.hpp"

#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

#include "admob_ios.hpp"

#else

#include "admob_null.hpp"

#endif

namespace admob {

static ek::StaticStorage<Context> _context;
Context& context = *_context.ptr();

void _initialize() {
    EK_DEBUG("admob initialize");
    _context.initialize();
}

void shutdown() {
    EK_DEBUG("admob shutdown");
    _context.shutdown();
}

}