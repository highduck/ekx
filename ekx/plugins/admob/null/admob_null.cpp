#include "admob.hpp"

namespace admob {

bool hasSupport() {
    return false;
}

void initialize(const config_t& config) {
    (void) config;
    _initialize();
}

void show_banner(int flags) {
    (void) flags;
}

void show_rewarded_ad() {

}

// returns `false` if interstitial is not loaded
void show_interstitial_ad() {
}

}