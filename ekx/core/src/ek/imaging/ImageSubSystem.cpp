#include "ImageSubSystem.hpp"
#include "../Allocator.hpp"
#include "../util/StaticStorage.hpp"
#include "../debug.hpp"

// Implementation headers
#include "Image_impl.hpp"
#include "Image_FastBlur.hpp"
#include "Image_Decode.hpp"
#include "Image_Drawing.hpp"

namespace ek::imaging {

struct PrivateState {
    ProxyAllocator allocator{"imaging"};
};

static StaticStorage<PrivateState> state{};
Allocator& allocator = state.ref().allocator;

void initialize() {
    EK_TRACE("imaging initialize");
    state.initialize();
}

void shutdown() {
    EK_TRACE("imaging shutdown");
    state.shutdown();
}

}