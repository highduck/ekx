#include "billing.hpp"
#include <ek/util/StaticStorage.hpp>
#include <ek/util/logger.hpp>

namespace billing {

static ek::StaticStorage<Context> _context{};
Context& context = *_context.ptr();

void _initialize() {
    EK_TRACE << "billing initialize";
    _context.initialize();
}

void shutdown() {
    EK_TRACE << "billing shutdown";
    _context.shutdown();
}

}