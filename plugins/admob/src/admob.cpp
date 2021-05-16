#include "admob.hpp"
#include <ek/util/StaticStorage.hpp>
#include <ek/debug.hpp>

namespace admob {

static ek::StaticStorage<Context> _context;
Context& context = *_context.ptr();

void _initialize() {
    EK_TRACE << "admob initialize";
    _context.initialize();
}

void shutdown() {
    EK_TRACE << "admob shutdown";
    _context.shutdown();
}

}