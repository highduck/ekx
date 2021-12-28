#ifndef EK_HANDLE_H
#define EK_HANDLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_handles_desc {
    uint16_t* sparse;
    uint16_t* dense;
    uint8_t* generations;
    uint32_t num;
} ek_handles_desc;

typedef struct ek_handles_data_layout{
    void* data;
    uint32_t stride;
} ek_handles_data_layout;

void ek_handles_init(ek_handles_desc h);

uint32_t ek_handles_new(ek_handles_desc h);

bool ek_handles_valid(ek_handles_desc h, uint32_t id);

void ek_handles_destroy(ek_handles_desc h, uint32_t id, ek_handles_data_layout layout);

uint16_t ek_handles__get_count(ek_handles_desc h);

uint16_t ek_handles__get_slot_index(ek_handles_desc h, uint16_t index);

//uint16_t ek_handles_get(ek_handles_desc h, uint32_t id);

#ifdef __cplusplus
}
#endif

#endif // EK_HANDLE_H
