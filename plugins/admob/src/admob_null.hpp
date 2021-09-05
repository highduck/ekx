#pragma once

namespace admob {

bool hasSupport() {
    return false;
}

void initialize(const Config& config) {
    _initialize();
    context.config = config;
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