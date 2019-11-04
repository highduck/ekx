#pragma once

#include <cassert>
#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>
#include "bit_count.hpp"

namespace ecxx {

template<typename T, const T NullValue = T(), const uint32_t PageSize = 0x8000u>
class sparse_vector {
public:
    // PageSize required to be power-of-two value
    static_assert(PageSize > 0u && ((PageSize & (PageSize - 1u)) == 0u));

    using size_type = uint32_t;
    using page_offset_type = uint16_t;
    using page_index_type = uint16_t;

    static constexpr uint32_t elements_per_page = PageSize / sizeof(T);
    static constexpr uint32_t page_mask = elements_per_page - 1u;
    static constexpr uint32_t page_bits = bit_count(page_mask);

    struct page_data {
        std::unique_ptr<T[]> elements;
        size_type count{0u};
    };

    void ensure(page_index_type page) {
        if (page >= pages_.size()) {
            pages_.resize(page + 1);
        }

        if (!pages_[page].elements) {
            pages_[page].elements = std::make_unique<T[]>(elements_per_page);
            std::fill_n(pages_[page].elements.get(), elements_per_page, NullValue);
        }
    }

    void insert(size_type i, T v) {
        assert(v != NullValue);
        assert(!has(i));

        page_index_type pi = i >> page_bits;
        page_offset_type po = i & page_mask;

        ensure(pi);
        pages_[pi].elements[po] = v;
        pages_[pi].count++;
    }

    void replace(size_type i, T v) {
        assert(v != NullValue);
        assert(has(i));

        page_index_type pi = i >> page_bits;
        page_offset_type po = i & page_mask;
        pages_[pi].elements[po] = v;
    }

    // unsafe just write element to map
//    void set(size_type i, T v) {
//        page_index_type pi = i >> page_bits;
//        page_offset_type po = i & page_mask;
//
//        ensure(pi);
//        pages_[pi].elements[po] = v;
//        pages_[pi].count++;
//    }

    T get_checked(size_type i) const {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        T el{NullValue};
        if (page < pages_.size()) {
            const page_data& p = pages_[page];
            if (p.count) {
                el = p.elements[offset];
            }
        }
        return el;
    }

    inline T at(size_type i) const {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        return pages_[page].elements[offset];
    }

    void remove(size_type i) {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        assert(has(page, offset));
        page_data& p = pages_[page];
        p.elements[offset] = NullValue;
        --p.count;
    }

    T get_and_remove(size_type i) {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        assert(has(page, offset));
        page_data& p = pages_[page];
        auto v = p.elements[offset];
        p.elements[offset] = NullValue;
        --p.count;
        return v;
    }

    [[nodiscard]] inline bool has(size_type i) const {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        return has(page, offset);
    }

    [[nodiscard]] inline bool has(page_index_type i, page_offset_type j) const {
        if (i < pages_.size()) {
            const page_data& p = pages_[i];
            return p.count && p.elements[j] != NullValue;
        } else {
            return false;
        }
    }

private:

    std::vector<page_data> pages_;
};

}