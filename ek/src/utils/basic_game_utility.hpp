#pragma once

#include <functional>
#include <string>
#include <ek/signals.hpp>

namespace ek {

class LeaderBoard {
public:

    explicit LeaderBoard(std::string id);

    void show() const;

    void submit(int score) const;

private:
    std::string id_;
};

class BasicGameUtility {
public:
    static void init();

    static void navigate(const std::string& url, bool blank);

    static void rateUs(const std::string& appId);

    static void showAchievements();

    static void updateAchievement(const std::string& code, int count);

    static bool isAdsRemoved();

    static signal_t<> onAdsRemoved;
    static signal_t<> onAdsRewarded;

    static void adsOnGameOver();

    static int unlockSite();

    static void adsShowVideo(const std::function<void(bool)>& callback);

    static void removeAds();

    static void shareScore(const std::string& text);

    static void shareApp(const std::string& text);
};

}
