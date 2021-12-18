#ifndef EK_GAME_SERVICES_H
#define EK_GAME_SERVICES_H

#ifdef __cplusplus
extern "C" {
#endif

void ek_game_services_init(void);

void ek_leaderboard_show(const char* id);

void ek_leaderboard_submit(const char* id, int score);

void ek_achievement_update(const char* id, int increment);

void ek_achievement_show(void);

#ifdef __cplusplus
}
#endif

#endif // EK_GAME_SERVICES_H
