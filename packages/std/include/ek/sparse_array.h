#ifndef EK_SPARSE_ARRAY_H
#define EK_SPARSE_ARRAY_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

    //16-bit
enum {
    // general 4kb pages
    EK_SPARSE_ARRAY_PAGE_SIZE = 0x1000,
    // total 0x800 elements per page
    EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE = EK_SPARSE_ARRAY_PAGE_SIZE / sizeof(uint16_t),
    // 0x7FF
    EK_SPARSE_ARRAY_PAGE_MASK = EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE - 1,
    // bits count for PAGE_MASK
    EK_SPARSE_ARRAY_PAGE_BITS = 11
};

typedef struct ek_sparse_array {
    void* data;
} ek_sparse_array;

typedef uint32_t ek_sparse_array_key;
typedef uint16_t ek_sparse_array_val;

ek_sparse_array ek_sparse_array_create(uint32_t num);
ek_sparse_array ek_sparse_array_offset(ek_sparse_array sa, int off);
void ek_sparse_array_clear(ek_sparse_array* sa, uint32_t num);
void ek_sparse_array_free(ek_sparse_array* sa, uint32_t num);

ek_sparse_array_val ek_sparse_array_get(ek_sparse_array sa, uint32_t key);

void ek_sparse_array_set(ek_sparse_array sa, ek_sparse_array_key key, ek_sparse_array_val val);

void ek_sparse_array_insert(ek_sparse_array sa, ek_sparse_array_key key, ek_sparse_array_val val);

ek_sparse_array_val ek_sparse_array_move_remove(ek_sparse_array sa, ek_sparse_array_key removed, ek_sparse_array_key target);

#ifdef __cplusplus
}
#endif

#endif // EK_SPARSE_ARRAY_H
