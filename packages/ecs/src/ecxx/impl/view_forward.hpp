#pragma once

#include "world.hpp"
#include <ek/ds/FixedArray.hpp>

namespace ecs {

template<typename ...Component>
class ViewForward {
public:
    static constexpr auto components_num = sizeof ... (Component);

    using table_index_type = uint32_t;
    using table_type = ComponentHeader* [components_num];

    class iterator final {
    public:
        iterator(const table_type& table, uint32_t it) noexcept: it_{it},
                                                                 table_{table} {
            if (!is_valid(it_, table_)) {
                ++(*this);
            }
        }

        iterator() noexcept = default;

        inline iterator& operator++() noexcept {
            return is_valid(++it_, table_) ? *this : ++(*this);
        }

        const iterator operator++(int) noexcept {
            iterator orig = *this;
            return ++(*this), orig;
        }

        inline bool operator==(const iterator& other) const {
            return it_ == other.it_;
        }

        inline bool operator!=(const iterator& other) const {
            return it_ != other.it_;
        }

        inline static bool is_valid(uint32_t it, const table_type& table) {
            // check primary entity vector end
            const ComponentHeader* m = table[0];
            if (it == m->handleToEntity.size()) {
                return true;
            }
            // filter secondary entity vectors
            const auto entity = m->handleToEntity.get(it);
            for (uint32_t k = 1u; k < components_num; ++k) {
                if (ek_sparse_array_get(table[k]->entityToHandle, entity) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline static bool is_valid_fast(entity_t idx, const table_type& table) {
            // filter secondary entity vectors
            const uint32_t cn = components_num;
            for (uint32_t k = 1u; k < cn; ++k) {
                if (ek_sparse_array_get(table[k]->entityToHandle, idx) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline EntityApi operator*() const noexcept {
            return EntityApi{table_[0]->handleToEntity.get(it_)};
        }

        inline EntityApi operator*() noexcept {
            return EntityApi{table_[0]->handleToEntity.get(it_)};
        }

    private:
        uint32_t it_{};
        const table_type& table_;
    };

    ViewForward() {
        {
            table_index_type i{};
            ((table_[i] = &C<Component>::header, ++i), ...);
        }

        qsort(table_, components_num, sizeof(table_[0]), ComponentHeader::compareBySize);
#ifndef NDEBUG
        {
            table_index_type i{};
            ((++C<Component>::header.debug_lock_counter, ++i), ...);
        }
#endif
    }

    ~ViewForward() {
#ifndef NDEBUG
        table_index_type i{};
        ((--C<Component>::header.debug_lock_counter, ++i), ...);
#endif
    }

    iterator begin() const {
        return {table_, 1u};
    }

    iterator end() const {
        return {table_, table_[0]->handleToEntity.size()};
    }

    template<typename Func>
    void each(Func func) const {
        const ComponentHeader* table_0 = table_[0];
        const auto size = table_0->handleToEntity.size();
#pragma nounroll
        for (uint32_t i = 1u; i != size; ++i) {
            const entity_t e = table_0->handleToEntity.get(i);
            if (iterator::is_valid_fast(e, table_)) {
                func(C<Component>::get(e)...);
            }
        }
    }

private:
    table_type table_;
};

template<typename Component>
class ViewForward<Component> {
public:
    static constexpr auto components_num = 1;

    class iterator final {
    public:
        iterator(uint32_t it) noexcept: it_{it} {
        }

        iterator() noexcept = default;

        inline iterator& operator++() noexcept {
            ++it_;
            return *this;
        }

        const iterator operator++(int) noexcept {
            iterator orig = *this;
            return ++(*this), orig;
        }

        inline bool operator==(const iterator& other) const {
            return it_ == other.it_;
        }

        inline bool operator!=(const iterator& other) const {
            return it_ != other.it_;
        }

        inline EntityApi operator*() const noexcept {
            return EntityApi{C<Component>::header.handleToEntity.get(it_)};
        }

    private:
        uint32_t it_{};
    };

    ViewForward() {
#ifndef NDEBUG
        ++C<Component>::header.debug_lock_counter;
#endif
    }

    ~ViewForward() {
#ifndef NDEBUG
        --C<Component>::header.debug_lock_counter;
#endif
    }

    iterator begin() const {
        return {1u};
    }

    iterator end() const {
        return {C<Component>::header.handleToEntity.size()};
    }

    template<typename Func>
    void each(Func func) const {
        const auto size = C<Component>::header.count();
#pragma nounroll
        for (uint32_t i = 1u; i != size; ++i) {
            func(C<Component>::get_by_handle(i));
        }
    }
};

}