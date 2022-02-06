#pragma once

#include "world.hpp"
#include <ek/ds/FixedArray.hpp>

namespace ecs {

template<typename ...Component>
class ViewForward {
public:
    static constexpr auto components_num = sizeof ... (Component);

    class iterator final {
    public:
        iterator(const ecx_component_type* const table[components_num], uint32_t it) noexcept: it_{it},
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

        inline static bool is_valid(uint32_t it, const ecx_component_type* const * table) {
            // check primary entity vector end
            const ecx_component_type* m = table[0];
            if (it == m->size) {
                return true;
            }
            // filter secondary entity vectors
            const auto entity = m->handleToEntity[it];
            for (uint32_t k = 1u; k < components_num; ++k) {
                if (ek_sparse_array_get(table[k]->entityToHandle, entity) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline static bool is_valid_fast(entity_t idx, const ecx_component_type** table) {
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
            return EntityApi{table_[0]->handleToEntity[it_]};
        }

        inline EntityApi operator*() noexcept {
            return EntityApi{table_[0]->handleToEntity[it_]};
        }

    private:
        uint32_t it_ = 0;
        const ecx_component_type* const* table_;
    };

    ViewForward() noexcept {
        {
            uint32_t i = 0;
            ((table_[i] = C<Component>::header, ++i), ...);
        }

        qsort(table_, components_num, sizeof(table_[0]), ecx_component_type_compare);
#ifndef NDEBUG
        {
            uint32_t i = 0;
            ((++C<Component>::header->lock_counter, ++i), ...);
        }
#endif
    }

    ~ViewForward() noexcept {
#ifndef NDEBUG
        uint32_t i = 0;
        ((--C<Component>::header->lock_counter, ++i), ...);
#endif
    }

    iterator begin() const {
        return {table_, 1u};
    }

    iterator end() const {
        return {table_, table_[0]->size};
    }

    template<typename Func>
    void each(Func func) const {
        const ecx_component_type* table_0 = table_[0];
        const uint16_t size = table_0->size;
#pragma nounroll
        for (uint16_t i = 1u; i != size; ++i) {
            const entity_t e = table_0->handleToEntity[i];
            if (iterator::is_valid_fast(e, table_)) {
                func(C<Component>::get(e)...);
            }
        }
    }

    ecx_component_type* table_[components_num];
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
            return EntityApi{C<Component>::header->handleToEntity[it_]};
        }

    private:
        uint32_t it_{};
    };

    ViewForward() noexcept {
#ifndef NDEBUG
        ++C<Component>::header->lock_counter;
#endif
    }

    ~ViewForward() noexcept {
#ifndef NDEBUG
        --C<Component>::header->lock_counter;
#endif
    }

    iterator begin() const {
        return {1u};
    }

    iterator end() const {
        return {C<Component>::header->size};
    }

    template<typename Func>
    void each(Func func) const {
        const uint16_t size = C<Component>::header->size;
#pragma nounroll
        for (uint32_t i = 1u; i != size; ++i) {
            func(C<Component>::get_by_handle(i));
        }
    }
};

}