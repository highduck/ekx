#include <ek/sparse_array.h>

typedef struct ek_sparse_array_page {
    // 4096 bytes, page aligned
    ek_sparse_array_val data[EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE];
} ek_sparse_array_page;

ek_sparse_array_page* invalid_page = 0;

static uint32_t ek_sparse_array__get_pages_num(uint32_t count) {
    return (count + (EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE - 1)) / EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE;
}

ek_sparse_array ek_sparse_array_create(const uint32_t num) {
    if (invalid_page == 0) {
        invalid_page = (ek_sparse_array_page*) calloc(1, sizeof(ek_sparse_array_page));
    }
    const uint32_t pages_count = ek_sparse_array__get_pages_num(num);
    ek_sparse_array_page** pages = (ek_sparse_array_page**) malloc(pages_count * sizeof(ek_sparse_array_page*));

    #pragma nounroll
    for (uint32_t i = 0; i < pages_count; ++i) {
        pages[i] = invalid_page;
    }

    return (ek_sparse_array) {.data = pages};
}

ek_sparse_array ek_sparse_array_offset(ek_sparse_array sa, const int off) {
    EK_ASSERT((off & (EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE - 1)) == 0);
    int po = off / EK_SPARSE_ARRAY_ELEMENTS_PER_PAGE;
    return (ek_sparse_array) {
            .data = ((ek_sparse_array_page**)sa.data) + po
    };
}

ek_sparse_array_val ek_sparse_array_get(const ek_sparse_array sa, const uint32_t key) {
    const ek_sparse_array_page** pages = (const ek_sparse_array_page**) sa.data;
    EK_ASSERT_R2(pages != 0);
    const ek_sparse_array_page* page = pages[key >> EK_SPARSE_ARRAY_PAGE_BITS];
    EK_ASSERT_R2(page != 0);
    return page->data[key & EK_SPARSE_ARRAY_PAGE_MASK];
}

void ek_sparse_array_set(ek_sparse_array sa, ek_sparse_array_key key, ek_sparse_array_val val) {
    ek_sparse_array_page** pages = (ek_sparse_array_page**) sa.data;
    EK_ASSERT_R2(pages != 0);
    ek_sparse_array_page* page = pages[key >> EK_SPARSE_ARRAY_PAGE_BITS];
    EK_ASSERT_R2(page != 0);
    EK_ASSERT_R2(page != invalid_page);
    page->data[key & EK_SPARSE_ARRAY_PAGE_MASK] = val;
}

void ek_sparse_array_insert(ek_sparse_array sa, ek_sparse_array_key key, ek_sparse_array_val val) {
    EK_ASSERT_R2(val != 0);
    ek_sparse_array_page** pages = (ek_sparse_array_page**) sa.data;
    EK_ASSERT_R2(pages != 0);
    const uint32_t page_i = key >> EK_SPARSE_ARRAY_PAGE_BITS;
    const uint32_t offset_i = key & EK_SPARSE_ARRAY_PAGE_MASK;
    ek_sparse_array_page* page = pages[page_i];
    EK_ASSERT_R2(page->data[offset_i] == 0);
    if (page != invalid_page) {
        page->data[offset_i] = val;
    } else {
        ek_sparse_array_page* new_page = (ek_sparse_array_page*) calloc(1, EK_SPARSE_ARRAY_PAGE_SIZE);
        new_page->data[offset_i] = val;
        pages[page_i] = new_page;
    }
}

// get `removed` element
// remove it at `removed` and assign to `target`
// returns handle
// `removed` and `target` should not be 0
ek_sparse_array_val
ek_sparse_array_move_remove(ek_sparse_array sa, ek_sparse_array_key removed, ek_sparse_array_key target) {
    ek_sparse_array_page** pages = (ek_sparse_array_page**) sa.data;
    EK_ASSERT_R2(pages != 0);

    // read: REMOVED entity data index (to relink DST entity to found index)
    const uint32_t page_i = removed >> EK_SPARSE_ARRAY_PAGE_BITS;
    ek_sparse_array_page* page = pages[page_i];
    const uint32_t offset_i = removed & EK_SPARSE_ARRAY_PAGE_MASK;
    const ek_sparse_array_val v = page->data[offset_i];

    // write: link removed entity to zero data (we need to check entity HAS data)
    EK_ASSERT_R2(v != 0);
    page->data[offset_i] = 0;

    // write: relink DST entity to REMOVED entity data index
    pages[target >> EK_SPARSE_ARRAY_PAGE_BITS]->data[target & EK_SPARSE_ARRAY_PAGE_MASK] = v;
    return v;
}

void ek_sparse_array_clear(ek_sparse_array* sa, const uint32_t num) {
    EK_ASSERT_R2(sa != 0);
    ek_sparse_array_page** pages = (ek_sparse_array_page**) sa->data;
    EK_ASSERT_R2(pages != 0);
    const uint32_t pages_count = ek_sparse_array__get_pages_num(num);
    for(uint32_t i = 0; i < pages_count; ++i) {
        ek_sparse_array_page* page = pages[i];
        if (page != invalid_page) {
            free(page);
            pages[i] = invalid_page;
        }
    }
}

void ek_sparse_array_free(ek_sparse_array* sa, const uint32_t num) {
    EK_ASSERT_R2(sa != 0);
    ek_sparse_array_clear(sa, num);
    free(sa->data);
    sa->data = 0;
}
