#ifndef EK_HANDLE_H
#define EK_HANDLE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// calculate total bytes required to store `capacity` handles map
#define ids_get_alloc_size(capacity) ((capacity) * (sizeof(uint32_t) + sizeof(uint16_t)))

typedef struct ids {
    // dense[0] = u16 info[2]
    // info[0] = count (always non-zero) (in case if capacity is MAX and pool is FULL, so count == 0 and capacity == 0 after overflow)
    // info[1] = capacity (you should keep in mind - it will be zero in case of MAX capacity (0x10000)), there is no true zero capacity at all
    uint32_t* dense;
    uint16_t* sparse;
} ids_t;

typedef struct id_data {
    void* data;
    uint32_t stride;
} id_data_t;

void ids_init(ids_t h, uint32_t capacity);

int ids_count(ids_t h);

uint32_t id_new(ids_t h);

bool id_valid(ids_t h, uint32_t id);

void id_destroy(ids_t h, uint32_t id, id_data_t layout);

#ifdef __cplusplus
}
#endif

#endif // EK_HANDLE_H
