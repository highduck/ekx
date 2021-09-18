#include "math/rand_impl.hpp"

#include "time/FpsMeter_impl.hpp"
#include "time/FrameTimer_impl.hpp"
#include "time/Stopwatch_impl.hpp"
#include "time/TimeLayer_impl.hpp"
#include "time/Timers_impl.hpp"
#include "time/Clock_impl.hpp"

#include "debug/LogSystem_impl.hpp"

#include "util/StringUtil_impl.hpp"
#include "util/Path_impl.hpp"
#include "util/Base64_impl.hpp"

namespace ek::core {

void setup() {
    LogSystem::initialize();
    Clock::setup();
    setupTimers();
}

}