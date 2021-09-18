#include <GameServices.hpp>
#include <ek/debug.hpp>

namespace ek {

void game_services_init() {
    EK_DEBUG("game-services initialize");
}

void leader_board_show(const char* id) {
    EK_DEBUG_F("game-services: show leaderboard: %s", id);
}

void leader_board_submit(const char* id, int score) {
    EK_DEBUG_F("game-services: submit to leaderboard: %s %d", id, score);
}

void achievement_update(const char* id, int score) {
    EK_DEBUG_F("game-services: achievement update: %s %d", id, score);
}

void achievement_show() {
    EK_DEBUG("game-services: show achievements");
}

}