#include "basic_game_utility.hpp"

#include <ek/util/locator.hpp>
#include <ek/ext/game_center/game_center.hpp>
#include <ek/ext/sharing/sharing.hpp>

namespace ek {

LeaderBoard::LeaderBoard(std::string id)
        : id_{std::move(id)} {

}

void LeaderBoard::show() const {
    leader_board_show(id_.c_str());
}

void LeaderBoard::submit(int score) const {
    leader_board_submit(id_.c_str(), score);
}

void BasicGameUtility::showAchievements() {
    achievement_show();
}

void BasicGameUtility::updateAchievement(const std::string& code, int count) {
    achievement_update(code.c_str(), count);
}

void BasicGameUtility::navigate(const std::string& url, bool blank) {
    (void) blank;
    sharing_navigate(url.c_str());
}

void BasicGameUtility::shareScore(const std::string& text) {
    sharing_send_message(text.c_str());
}

void BasicGameUtility::shareApp(const std::string& text) {
    sharing_send_message(text.c_str());
}

void BasicGameUtility::rateUs(const std::string& appId) {
    sharing_rate_us(appId.c_str());
}

int BasicGameUtility::unlockSite() { return 1; }

}
