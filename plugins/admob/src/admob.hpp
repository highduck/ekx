#pragma once

#include <functional>
#include <string>
#include <ek/util/signals.hpp>

namespace admob {

enum class event_type {
    initialized = 0,
    video_rewarded = 1,
    video_loaded = 2,
    video_failed = 3,
    video_loading = 4,
    video_closed = 5
};

enum class ChildDirected {
    Unspecified = -1,
    False = 0,
    True = 1
};

struct config_t {
    std::string banner;
    std::string video;
    std::string inters;
    ChildDirected childDirected = ChildDirected::Unspecified;
};

struct Context {
    ek::signal_t<event_type> onEvent;
    ek::signal_t<> onInterstitialClosed;
    config_t config;
};

extern Context& context;

void initialize(const config_t& config);

void _initialize();
void shutdown();

void show_banner(int flags);

void show_rewarded_ad();

void show_interstitial_ad();

bool hasSupport();

}
