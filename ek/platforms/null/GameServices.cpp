#include "platform/game_center.hpp"

namespace ek {

    void game_services_init() {}

    void leader_board_show(const char* leader_board_id) {
        (void) leader_board_id;
    }

    void leader_board_submit(const char* leader_board_id, int score) {
        (void) leader_board_id;
        (void) score;
    }

    void achievement_update(const char* achievement_id, int increment) {
        (void) achievement_id;
        (void) increment;
    }

    void achievement_show() {}
}