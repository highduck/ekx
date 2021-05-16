#include <GameServices.hpp>
#include <ek/debug.hpp>

namespace ek {

void game_services_init() {
    EK_TRACE << "game-services initialize";
}

void leader_board_show(const char* id) {
    EK_TRACE << "game-services: show leaderboard: " << id;
}

void leader_board_submit(const char* id, int score) {
    EK_TRACE << "game-services: submit to leaderboard: " << id << " " << score;
}

void achievement_update(const char* id, int score) {
    EK_TRACE << "game-services: achievement update: " << id << " " << score;
}

void achievement_show() {
    EK_TRACE << "game-services: show achievements";
}

}