#pragma once

#include <cassert>
#include <vector>
#include <cstdint>
#include <memory>
#include <algorithm>
#include "utility.hpp"

namespace ecs {

class sparse_vector {
public:
    // PageSize required to be power-of-two value

    using size_type = uint32_t;
    using page_offset_type = uint16_t;
    using page_index_type = uint16_t;

    using T = uint32_t;

    static constexpr uint32_t null_value = 0;
    static constexpr uint32_t page_size = 0x8000u / sizeof(T);
    static_assert(page_size > 0u && ((page_size & (page_size - 1u)) == 0u));

    static constexpr uint32_t elements_per_page = page_size / sizeof(T);
    static constexpr uint32_t page_mask = elements_per_page - 1u;
    static constexpr uint32_t page_bits = details::bit_count(page_mask);

    struct page_data {
        std::unique_ptr<T[]> elements;
        size_type count{0u};
    };

    page_data& ensure(page_index_type page) {
        if (page >= pages_.size()) {
            pages_.resize(page + 1);
        }

        auto& result = pages_[page];
        if (!result.elements) {
            result.elements = std::make_unique<T[]>(elements_per_page);
            std::fill_n(result.elements.get(), elements_per_page, null_value);
        }
        return result;
    }

    void insert(size_type i, T v) {
        assert(v != null_value);
        assert(!has(i));

        page_index_type pi = i >> page_bits;
        page_offset_type po = i & page_mask;

        auto& page = ensure(pi);
        page.elements[po] = v;
        page.count++;
    }

    void replace(size_type i, T v) {
        assert(v != null_value);
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

    [[nodiscard]]
    T get_checked(size_type i) const {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        T el{null_value};
        if (page < pages_.size()) {
            const page_data& p = pages_[page];
            if (p.count) {
                el = p.elements[offset];
            }
        }
        return el;
    }

    [[nodiscard]]
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
        p.elements[offset] = null_value;
        --p.count;
    }

    T get_and_remove(size_type i) {
        const page_index_type page = i >> page_bits;
        const page_offset_type offset = i & page_mask;
        assert(has(page, offset));
        page_data& p = pages_[page];
        auto v = p.elements[offset];
        p.elements[offset] = null_value;
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
            return p.count && p.elements[j] != null_value;
        } else {
            return false;
        }
    }

    void clear() {
        pages_.clear();
    }

private:
    std::vector<page_data> pages_;
};

}