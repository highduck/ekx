#include "billing.hpp"
#include <ek/util/StaticStorage.hpp>
#include <ek/log.h>

// implementation
#if defined(__APPLE__)

#include <TargetConditionals.h>

#endif

#if defined(__ANDROID__)

#include "billing_android.hpp"

#elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR

#include "billing_ios.hpp"

#else

#include "billing_sim.hpp"

#endif

namespace billing {

static ek::StaticStorage<Context> _context{};
Context& context = *_context.ptr();

void _initialize() {
    log_debug("billing initialize");
    _context.initialize();
}

void shutdown() {
    log_debug("billing shutdown");
    _context.shutdown();
}

}