#include "math/Random.hpp"

#include "time/Timers_impl.hpp"
#include "util/Res.hpp"

#include <ek/log.h>
#include <ek/time.h>

namespace ek::core {

void setup() {
    void* lt = malloc(4);
    (void)lt;

    ResourceDB::instance.initialize();
    log_init();
    ek_time_init();
    setupTimers();

    auto seed = ek_time_seed32();
    rand_default.setSeed(seed++);
    rand_fx.setSeed(seed++);
    rand_game.setSeed(seed++);
}

}

////#ifdef EK_ALLOCATION_TRACKER
//
//void* operator new(size_t sz) {
//    if (sz == 0) {
//        ++sz;
//    }
//
//    if (void* ptr = std::malloc(sz)) {
//        memset(ptr, 0xCC, sz);
//        return ptr;
//    }
//
//    abort();
//}
//
//void operator delete(void* ptr) noexcept {
//    std::free(ptr);
//}

////#endif // EK_ALLOCATION_TRACKER