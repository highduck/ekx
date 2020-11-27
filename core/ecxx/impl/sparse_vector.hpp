#pragma once

#include "utility.hpp"
#include <vector>
#include <memory>

namespace ecs {

class sparse_vector {
public:
    using size_type = unsigned;
    using page_offset_type = unsigned short;
    using page_index_type = unsigned short;

    using T = unsigned;

    static constexpr unsigned null_value = 0;
    static constexpr unsigned page_size = 0x8000u; // ~32kb index table per page
    static constexpr unsigned elements_per_page = page_size / sizeof(T);
    static constexpr unsigned page_mask = 0x1FFFu; // (elements_per_page - 1)
    static constexpr unsigned page_bits = 13; // bit_count of page_mask

    // `page_size` required to be power-of-two value
    static_assert(elements_per_page > 0u && ((elements_per_page & (elements_per_page - 1u)) == 0u));

    struct page_data {
        std::unique_ptr<T[]> elements;
        size_type count = 0u;
    };

    page_data& ensure(page_index_type page) {
        if (page >= pagesCount_) {
            pagesCount_ = page + 1;
            pages_.resize(pagesCount_);
        }

        auto& result = pages_[page];
        if (!result.elements) {
            result.elements = std::make_unique<T[]>(elements_per_page);
            auto* ptr = result.elements.get();
            for (unsigned i = 0; i < elements_per_page; ++i) {
                ptr[i] = 0;
            }
        }
        return result;
    }

    void insert(size_type i, T v) {
        ECXX_FULL_ASSERT(v != null_value);
        ECXX_FULL_ASSERT(!has(i));

        auto& page = ensure(i >> page_bits);
        page.elements[i & page_mask] = v;
        ++page.count;
    }

    void replace(size_type i, T v) {
        ECXX_FULL_ASSERT(v != null_value);
        ECXX_FULL_ASSERT(has(i));

        pages_[i >> page_bits].elements[i & page_mask] = v;
    }

    [[nodiscard]]
    inline T at(size_type i) const {
        return pages_[i >> page_bits].elements[i & page_mask];
    }

    void remove(size_type i) {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        ECXX_FULL_ASSERT(has(page, offset));
        page_data& p = pages_[page];
        p.elements[offset] = null_value;
        --p.count;
    }

    T get_and_remove(size_type i) {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        ECXX_FULL_ASSERT(has(page, offset));
        page_data& p = pages_[page];
        auto v = p.elements[offset];
        p.elements[offset] = null_value;
        --p.count;
        return v;
    }

    [[nodiscard]] inline bool has(size_type i) const {
        return has(i >> page_bits, i & page_mask);
    }

    [[nodiscard]] inline bool has(page_index_type i, page_offset_type j) const {
        return i < pagesCount_ && pages_[i].count && pages_[i].elements[j] != null_value;
    }

    void clear() {
        pages_.clear();
        pagesCount_ = 0;
    }

private:
    std::vector<page_data> pages_;
    size_type pagesCount_ = 0;
};

}