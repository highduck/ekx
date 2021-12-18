#include <ek/sparse_array.h>

static ek_sparse_array_val* ek_sparse_array__alloc_data(const uint32_t num) {
    return (ek_sparse_array_val*)calloc(1, num * sizeof(ek_sparse_array_val));
}

ek_sparse_array ek_sparse_array_create(const uint32_t num) {
    return (ek_sparse_array) {
        .data = ek_sparse_array__alloc_data(num)
    };
}

ek_sparse_array ek_sparse_array_offset(ek_sparse_array sa, const int off) {
    return (ek_sparse_array) {
            .data = ((ek_sparse_array_val*)sa.data) + off
    };
}

ek_sparse_array_val ek_sparse_array_get(ek_sparse_array sa, uint32_t key) {
    ek_sparse_array_val* data = (ek_sparse_array_val*) sa.data;
    EK_ASSERT_R2(data != 0);
    return data[key];
}

void ek_sparse_array_set(ek_sparse_array sa, ek_sparse_array_key key, ek_sparse_array_val val) {
    ek_sparse_array_val* data = (ek_sparse_array_val*) sa.data;
    EK_ASSERT_R2(data != 0);
    data[key] = val;
}

void ek_sparse_array_insert(ek_sparse_array sa, ek_sparse_array_key key, ek_sparse_array_val val) {
    ek_sparse_array_val* data = (ek_sparse_array_val*) sa.data;
    EK_ASSERT_R2(data != 0);
    data[key] = val;
}

// get `removed` element
// remove it at `removed` and assign to `target`
// returns handle
// `removed` and `target` should not be 0
ek_sparse_array_val
ek_sparse_array_move_remove(ek_sparse_array sa, ek_sparse_array_key removed, ek_sparse_array_key target) {
    ek_sparse_array_val* data = (ek_sparse_array_val*) sa.data;
    EK_ASSERT_R2(data != 0);
    const ek_sparse_array_val v = data[removed];
    data[removed] = 0;
    data[target] = v;
    return v;
}

void ek_sparse_array_clear(ek_sparse_array* sa, const uint32_t num) {
    ek_sparse_array_free(sa, num);
    sa->data = ek_sparse_array__alloc_data(num);
}

void ek_sparse_array_free(ek_sparse_array* sa, const uint32_t num) {
    EK_ASSERT_R2(sa != 0);
    EK_ASSERT_R2(sa->data != 0);
    free(sa->data);
    sa->data = 0;
}