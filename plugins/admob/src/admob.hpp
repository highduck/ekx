#pragma once

#include <functional>
#include <string>
#include <ek/util/Signal.hpp>

namespace admob {

enum class EventType {
    Initialized = 0,
    VideoRewarded = 1,
    VideoLoaded = 2,
    VideoFailed = 3,
    VideoLoading = 4,
    VideoClosed = 5
};

enum class ChildDirected {
    Unspecified = -1,
    False = 0,
    True = 1
};

struct Config {
    std::string banner;
    std::string video;
    std::string inters;
    ChildDirected childDirected = ChildDirected::Unspecified;
};

struct Context {
    ek::Signal<EventType> onEvent;
    ek::Signal<> onInterstitialClosed;
    Config config;
};

extern Context& context;

void initialize(const Config& config);

void _initialize();
void shutdown();

void show_banner(int flags);

void show_rewarded_ad();

void show_interstitial_ad();

bool hasSupport();

}
