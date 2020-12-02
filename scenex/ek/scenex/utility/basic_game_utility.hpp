#pragma once

#include <functional>
#include <string>
#include <ek/util/signals.hpp>

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
    static void showAchievements();

    static void updateAchievement(const std::string& code, int count);
};

}
