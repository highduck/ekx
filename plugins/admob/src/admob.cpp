#include "admob.hpp"

namespace admob {
ek::signal_t<event_type> onEvent{};
ek::signal_t<> onInterstitialClosed{};
ek::signal_t<bool> onFocusChanged{};
}