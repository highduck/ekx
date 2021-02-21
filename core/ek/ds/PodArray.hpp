#pragma once

#include <ek/assert.hpp>
#include <stdlib.h>

namespace ek {

template<typename T, unsigned MaxCount>
struct SmallArray {

    static_assert(MaxCount <= 0x10000);

    unsigned size = 0;
    T data[MaxCount];

    inline void push(T el) {
        EK_ASSERT_R2(size < MaxCount);
        data[size++] = el;
    }
};

template<typename T>
struct DynArray {

    unsigned capacity;
    unsigned size;
    T* data;

    DynArray() {
        size = 0;
        capacity = 64;
        data = (T*) malloc(sizeof(T) * capacity);
    }

    ~DynArray() {
        free(data);
    }

    [[nodiscard]]
    inline bool empty() const {
        return size == 0;
    }

    inline void clear() {
        size = 0;
    }

    void grow() {
        capacity = capacity << 1;
        data = (T*) realloc(data, sizeof(T) * capacity);
    }

    inline void push_back(T el) {
        if (size == capacity) {
            grow();
        }
        *(data + size) = el;
        ++size;
    }

    inline void set(unsigned i, T el) const {
        EK_ASSERT_R2(i < size);
        *(data + i) = el;
    }

    [[nodiscard]]
    inline T get(unsigned i) const {
        EK_ASSERT_R2(i < size);
        return *(data + i);
    }

    inline void remove_back() {
        --size;
    }

    [[nodiscard]]
    inline T back() const {
        return *(data + size - 1);
    }
};


/** Sparse Array: map Entity to Component Handle **/

// min memory required:
// mem per vector = 32 (PagesCountMax) * 8 (ptr to page array) = 256 bytes
// for 128 components = 256 * 128 = 32768 bytes

// full page data size = 2048 (elements per page) * 2 (index size) = 4096 bytes
// all pages = 32 (pages for 65k entities) * 4096 = 131072 bytes

// FULL INDEX DATA memory for 128 component managers:
// (all pages + init mem) * 128 comps = (256 + 131072) * 128 = 16 809 984, 16 MB

// 0 is invalid index (null_value)

// 32 pages per component, so how many components at all we can track if pages will have global U16 id:
// - 65536 / 32 = 2048 components!

// size: 256 bytes
template<typename K, typename V, unsigned MaxCount>
struct SparseArray {
    //inline constexpr uint32_t SS_PAGE_SIZE = 0x8000; // ~32kb index table per page
    static constexpr unsigned PageSize = 0x1000; // general 4kb pages
    static constexpr unsigned ElementsPerPage = PageSize / sizeof(V); // 0x800 = 2048
    static constexpr unsigned PageMask = ElementsPerPage - 1; // 0x7FF
    static constexpr unsigned PageBits = 11; // bit count for (PageSize - 1)

    // count on 32 pages for 16-bit entity index
    static constexpr unsigned PagesMaxCount = MaxCount / ElementsPerPage;

    // `PageSize` required to be power-of-two value
    static_assert(ElementsPerPage > 0 && ((ElementsPerPage & (ElementsPerPage - 1)) == 0));

    struct Page {
        // 4096 bytes, page aligned
        V indices[ElementsPerPage];
    };

    static Page sInvalidPage;

    // 32 * 8 = 256
    Page* pages[PagesMaxCount];

    [[nodiscard]]
    inline V get(K k) const {
        return pages[k >> PageBits]->indices[k & PageMask];
    }

    inline void set(K k, V v) {
        pages[k >> PageBits]->indices[k & PageMask] = v;
    }

    void insert(K k, V v) {
        EK_ASSERT_R2(v != 0);
        auto* pages_ = pages;
        const auto page = k >> PageBits;
        const auto offset = k & PageMask;
        auto* indices = pages_[page]->indices;
        EK_ASSERT_R2(pages_[page]->indices[offset] == 0);
        if (indices != sInvalidPage.indices) {
            indices[offset] = v;
        } else {
            auto* pageData = (Page*) calloc(ElementsPerPage, sizeof(V));
            pageData->indices[offset] = v;
            pages_[page] = pageData;
        }
    }

    // get `removed` element
    // remove it at `removed` and assign to `target`
    // returns handle
    // `removed` and `target` should not be 0
    [[nodiscard]]
    V moveRemove(K removed, K target) {
        auto* pages_ = pages;

        // read: REMOVED entity data index (to relink DST entity to found index)
        const auto page = removed >> PageBits;
        auto* indices = pages_[page]->indices;
        const auto offset = removed & PageMask;
        const auto v = indices[offset];

        // write: link removed entity to zero data (we need to check entity HAS data)
        EK_ASSERT_R2(v != 0);
        indices[offset] = 0;

        // write: relink DST entity to REMOVED entity data index
        pages_[target >> PageBits]->indices[target & PageMask] = v;

        return v;
    }

    inline void init() {
        initArrays(this, 1);
    }

    inline void clear() {
        clearArrays(this, 1);
    }

    static void initArrays(SparseArray* arrays, unsigned count) {
        auto* pInvalidPage = &sInvalidPage;
        for (unsigned j = 0; j < count; ++j) {
            auto* pages = arrays[j].pages;
            for (unsigned i = 0; i < PagesMaxCount; ++i) {
                pages[i] = pInvalidPage;
            }
        }
    }

    static void clearArrays(SparseArray* arrays, unsigned count) {
        auto* pInvalidPage = &sInvalidPage;
        for (unsigned i = 0; i < count; ++i) {
            auto* pages = arrays[i].pages;
            for (unsigned j = 0; j < PagesMaxCount; ++j) {
                auto* page = pages[j];
                if (page != pInvalidPage) {
                    free(page);
                    pages[j] = pInvalidPage;
                }
            }
        }
    }
};

template<typename K, typename V, unsigned MaxCount>
inline typename SparseArray<K, V, MaxCount>::Page SparseArray<K, V, MaxCount>::sInvalidPage{};

}