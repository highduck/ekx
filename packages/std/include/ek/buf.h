#ifndef EK_BUF_H
#define EK_BUF_H

#include <ek/pre.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ek_buf_header_t {
    uint32_t pad0;
    uint32_t pad1;
    uint32_t capacity;
    uint32_t length;
} ek_buf_header_t;

ek_buf_header_t* ek_buf_header(const void* ptr);

uint32_t ek_buf_capacity(const void* ptr);

uint32_t ek_buf_length(const void* ptr);

bool ek_buf_full(const void* ptr);

bool ek_buf_empty(const void* ptr);

void ek_buf_reset(void** ptr);

void ek_buf_set_capacity(void** ptr, uint32_t newCapacity, uint32_t elementSize);

void ek_buf_set_size(void** buf, uint32_t elementSize, uint32_t len, uint32_t cap);

void* ek_buf_add_(void* ptr, uint32_t elementSize);

void* ek_buf_remove_(void* ptr, uint32_t i, uint32_t elementSize);

void* ek_buf_pop_(void* ptr, uint32_t elementSize);

// pod array impl
void arr_init_from(void** arr, uint32_t element_size, const void* src, uint32_t len);

void arr_resize(void** p_arr, uint32_t element_size, uint32_t new_len);

void arr_grow(void** p_arr, uint32_t capacity, uint32_t element_size);

void arr_maybe_grow(void** p_arr, uint32_t element_size);

void* arr_push_mem(void** p_arr, uint32_t element_size, const void* src);

void arr_assign(void** p_arr, uint32_t element_size, void* src_arr);

void arr_remove(void* arr, uint32_t element_size, uint32_t at);

void arr_swap_remove(void* arr, uint32_t element_size, uint32_t at);

void* arr_search(void* arr, uint32_t element_size, const void* el);

void arr_pop(void* arr);

/**
 * internal function ensure we have space for one more element in array
 * @param p_arr - pointer to buffer
 * @param element_size - array's element size
 * @return pointer to slot we can assign new element
 */
void* arr_add_(void** p_arr, uint32_t element_size);

#define arr_push(p_arr, T, el) (*((T*)arr_add_((void**)(p_arr), sizeof(T))) = (el))

void* _check_ptr_alignment(void* ptr, uint32_t width);

#define cast_ptr_aligned(T, ptr) ((T*)_check_ptr_alignment(ptr, sizeof(T)))

#ifdef __cplusplus
}
#endif

#endif // EK_BUF_H
