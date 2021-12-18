#include <ek/game_services.h>
#include <ek/app_native.h>
#include <ek/log.h>

#if defined(__APPLE__) && (TARGET_OS_IOS || TARGET_OS_TV)
#include "ek_game_services_ios.c.h"
#elif defined(__ANDROID__)
#include "ek_game_services_android.c.h"
#else

void ek_game_services_init(void) {
    log_debug("game-services initialize");
}

void ek_leaderboard_show(const char* id) {
    log_debug("game-services: show leaderboard: %s", id);
}

void ek_leaderboard_submit(const char* id, int score) {
    log_debug("game-services: submit to leaderboard: %s %d", id, score);
}

void ek_achievement_update(const char* id, int score) {
    log_debug("game-services: achievement update: %s %d", id, score);
}

void ek_achievement_show(void) {
    log_debug("game-services: show achievements");
}

#endif