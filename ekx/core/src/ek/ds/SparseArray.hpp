#pragma once

#include "../assert.hpp"
#include "../Allocator.hpp"

namespace ek {

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

    explicit SparseArray() : pages{} {
    }

    [[nodiscard]]
    inline V get(K k) const {
        return pages[k >> PageBits]->indices[k & PageMask];
    }

    inline void set(K k, V v) {
        pages[k >> PageBits]->indices[k & PageMask] = v;
    }

    void insert(K k, V v, Allocator& allocator) {
        EK_ASSERT_R2(v != 0);
        auto* pages_ = pages;
        const auto page = k >> PageBits;
        const auto offset = k & PageMask;
        auto* indices = pages_[page]->indices;
        EK_ASSERT_R2(pages_[page]->indices[offset] == 0);
        if (indices != sInvalidPage.indices) {
            indices[offset] = v;
        } else {
            AllocatorTraceScope allocatorTrace{"SparseArray::Page"};
            auto* pageData = (Page*) allocator.alloc(PageSize, PageSize);
            memory::clear(pageData, PageSize);
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

    inline void clear(Allocator& allocator) {
        clearArrays(this, 1, allocator);
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

    static void clearArrays(SparseArray* arrays, unsigned count, Allocator& allocator) {
        auto* pInvalidPage = &sInvalidPage;
        for (unsigned i = 0; i < count; ++i) {
            auto* pages = arrays[i].pages;
            for (unsigned j = 0; j < PagesMaxCount; ++j) {
                auto* page = pages[j];
                if (page != pInvalidPage) {
                    allocator.dealloc(page);
                    pages[j] = pInvalidPage;
                }
            }
        }
    }
};

template<typename K, typename V, unsigned MaxCount>
inline typename SparseArray<K, V, MaxCount>::Page SparseArray<K, V, MaxCount>::sInvalidPage{};

}