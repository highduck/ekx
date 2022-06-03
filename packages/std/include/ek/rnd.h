#ifndef EK_RND_H
#define EK_RND_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ek_rand1(uint32_t* state);
uint32_t ek_rand2(uint64_t* state);

uint32_t rnd_n(uint32_t* state, uint32_t count);
float rnd_f(uint32_t* state);
float rnd_range_f(uint32_t* state, float a, float b);
int rnd_range_i(uint32_t* state, int a, int b);

void rnd_shuffle(uint32_t* state, void* arr, uint32_t size, uint32_t item_size);
#define RND_ITEM(p_state, arr) ((arr)[rnd_n((p_state), sizeof(arr) / sizeof((arr)[0]))])

extern uint32_t random_seed;

uint32_t random_next(void);
uint32_t random_n(uint32_t count);
float random_f(void);
float random_range_f(float a, float b);
int random_range_i(int a, int b);
bool random_chance(float prob);

extern uint32_t game_random_seed;

uint32_t game_random_next(void);
uint32_t game_random_n(uint32_t count);
float game_random_f(void);
float game_random_range_f(float a, float b);
int game_random_range_i(int a, int b);
bool game_random_chance(float prob);

#ifdef __cplusplus
}
#endif

#include "rnd.hpp"

#endif // EK_RND_H
