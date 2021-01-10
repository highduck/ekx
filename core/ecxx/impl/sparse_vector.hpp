#pragma once

#include "utility.hpp"
#include <vector>
#include <memory>

namespace ecs {

class sparse_vector {
public:
    using size_type = uint32_t;
    using page_offset_type = uint16_t;
    using page_index_type = uint16_t;
    using T = uint32_t;

    static constexpr uint32_t null_value = 0;
    static constexpr uint32_t page_size = 0x8000u; // ~32kb index table per page
    static constexpr uint32_t elements_per_page = page_size / sizeof(T);
    static constexpr uint32_t page_mask = 0x1FFFu; // (elements_per_page - 1)
    static constexpr uint32_t page_bits = 13; // bit_count of page_mask

    static constexpr uint32_t ElementCountMax = 0x10000;
    static constexpr uint32_t PagesCountMax = ElementCountMax / elements_per_page;

    // `page_size` required to be power-of-two value
    static_assert(elements_per_page > 0u && ((elements_per_page & (elements_per_page - 1u)) == 0u));

    ~sparse_vector() {
        clear();
    }

    T* ensure(page_index_type page) {
        auto* elements = pageElements_[page];
        if (!elements) {
            pageElements_[page] = elements = new T[elements_per_page]();
        }
        return elements;
    }

    void insert(size_type i, T v) {
        ECXX_FULL_ASSERT(v != null_value);
        ECXX_FULL_ASSERT(pageElements_[i >> page_bits] == nullptr ||
                         pageElements_[i >> page_bits][i & page_mask] == null_value);
        ensure(i >> page_bits)[i & page_mask] = v;
    }

    void replace(size_type i, T v) {
        ECXX_FULL_ASSERT(v != null_value);
        ECXX_FULL_ASSERT(pageElements_[i >> page_bits] != nullptr);
        ECXX_FULL_ASSERT(pageElements_[i >> page_bits][i & page_mask] != null_value);

        pageElements_[i >> page_bits][i & page_mask] = v;
    }

    [[nodiscard]]
    inline T at(size_type i) const {
        return pageElements_[i >> page_bits][i & page_mask];
    }

    // returns mapped index or 0
    [[nodiscard]] inline T opt(size_type i) const {
        const auto* elements = pageElements_[i >> page_bits];
        return elements != nullptr ? elements[i & page_mask] : null_value;
    }

    void remove(size_type i) {
        const auto page = i >> page_bits;
        const auto offset = i & page_mask;
        ECXX_FULL_ASSERT(pageElements_[page] != nullptr);
        ECXX_FULL_ASSERT(pageElements_[page][offset] != null_value);
        pageElements_[page][offset] = null_value;
    }

    T get_and_remove(size_type i) {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        ECXX_FULL_ASSERT(has(page, offset));
        auto* elements = pageElements_[page];
        auto v = elements[offset];
        elements[offset] = null_value;
        return v;
    }

    // get `src` element
    // if src == dst - remove it from `src`
    // else - remove it at `src` and assign to `dst`
    // returns element
    // src and dst should be not null
    T moveRemove(size_type src, size_type dst) {
        const auto page = src >> page_bits;
        const auto offset = src & page_mask;
        auto* elements = pageElements_[page];
        ECXX_FULL_ASSERT(elements != nullptr);
        const auto v = elements[offset];
        ECXX_FULL_ASSERT(v != null_value);
        elements[offset] = null_value;
        if (src != dst) {
            replace(dst, v);
        }
        return v;
    }

    [[nodiscard]] inline bool has(size_type i) const {
        const auto* elements = pageElements_[i >> page_bits];
        return elements && elements[i & page_mask] != null_value;
    }

    [[nodiscard]] inline bool has(page_index_type i, page_offset_type j) const {
        const auto* elements = pageElements_[i];
        return elements && elements[j] != null_value;
    }

    void clear() {
        for (uint32_t i = 0; i < PagesCountMax; ++i) {
            delete[] pageElements_[i];
            pageElements_[i] = nullptr;
        }
    }

private:
    T* pageElements_[PagesCountMax]{};
};

}