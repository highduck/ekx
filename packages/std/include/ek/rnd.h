#ifndef EK_RND_H
#define EK_RND_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

uint32_t ek_rand1(uint32_t* state);
uint32_t ek_rand2(uint64_t* state);
float ek_norm_f32_from_u32(uint32_t x);

#ifdef __cplusplus
}
#endif

#endif // EK_RND_H
