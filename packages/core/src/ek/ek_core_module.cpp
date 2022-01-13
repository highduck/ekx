#include <ek/log.h>
#include <ek/time.h>
#include <ek/rnd.h>

namespace ek::core {

void setup() {
    //ResourceDB::init();
    log_init();
    ek_time_init();
    ek_timers_init();

    uint32_t seed = ek_time_seed32();
    random_seed = seed++;
    game_random_seed = seed;
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