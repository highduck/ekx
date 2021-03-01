#include "ImageSubSystem.hpp"
#include "../Allocator.hpp"
#include "../util/StaticStorage.hpp"

namespace ek::imaging {

struct PrivateState {
    ProxyAllocator allocator{"imaging"};
};

static StaticStorage<PrivateState> state{};
Allocator& allocator = state.ptr()->allocator;

void initialize() {
    state.initialize();
}

void shutdown() {
    state.shutdown();
}

}