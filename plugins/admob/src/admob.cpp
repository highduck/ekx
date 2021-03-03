#include "admob.hpp"
#include <ek/util/StaticStorage.hpp>

namespace admob {

static ek::StaticStorage<Context> _context;
Context& context = *_context.ptr();

void _initialize() {
    _context.initialize();
}

void shutdown() {
    _context.shutdown();
}

}