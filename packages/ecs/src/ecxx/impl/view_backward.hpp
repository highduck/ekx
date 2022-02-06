#pragma once

#include "world.hpp"
#include <ek/ds/FixedArray.hpp>
#include <cstdlib>

namespace ecs {

template<typename ...Component>
class ViewBackward {
public:
    static constexpr auto components_num = sizeof ... (Component);

    class iterator final {
    public:
        iterator(const ecx_component_type* const table[components_num], entity_t it) noexcept: it_{it},
                                                                                               ent_{0},
                                                                                               table_{table} {
            skips();
        }

        inline iterator& operator++() noexcept {
            --it_;
            skips();
            return *this;
        }

        inline bool operator==(const iterator& other) const {
            return ent_ == other.ent_;
        }

        inline bool operator!=(const iterator& other) const {
            return ent_ != other.ent_;
        }

        inline void skips() {
            // todo: size recovery (case we remove entities before *it)
            EK_ASSERT_R2(it_ < table_[0]->size);

            while (it_ != 0 && !valid(table_[0]->handleToEntity[it_])) {
                --it_;
            }
            ent_ = table_[0]->handleToEntity[it_];
        }

        [[nodiscard]]
        inline bool valid(entity_t e) const {
            for (uint32_t i = 1u; i < components_num; ++i) {
                if (ek_sparse_array_get(table_[i]->entityToHandle, e) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline EntityApi operator*() const {
            return EntityApi{ent_};
        }

        inline EntityApi operator*() {
            return EntityApi{ent_};
        }

    private:
        entity_t it_;
        entity_t ent_;
        const ecx_component_type* const* table_;
    };

    ViewBackward() noexcept {
        {
            uint32_t i = 0;
            ((table_[i] = C<Component>::header, ++i), ...);
        }
        qsort(table_, components_num, sizeof(table_[0]), ecx_component_type_compare);
    }

    iterator begin() const {
        return {table_, (entity_t) (table_[0]->size - 1)};
    }

    iterator end() const {
        return {table_, 0};
    }

    template<typename Func>
    void each(Func func) const {
#pragma nounroll
        for (auto e: *this) {
            func(*C<Component>::get_by_entity(e.index)...);
        }
    }

private:
    ecx_component_type* table_[components_num];
};

template<typename Component>
class ViewBackward<Component> {
public:
    static constexpr auto components_num = 1;

    class iterator final {
    public:
        iterator(uint32_t it) noexcept: it_{it} {
        }

        iterator() noexcept = default;

        inline iterator& operator++() noexcept {
            --it_;
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
        uint32_t it_ = 0;
    };

    ViewBackward() noexcept = default;

    iterator begin() const {
        const uint32_t sz = C<Component>::header->size;
        return {sz > 0 ? (sz - 1u) : 0u};
    }

    iterator end() const {
        return {0u};
    }

    template<typename Func>
    void each(Func func) const {
#pragma nounroll
        for (uint32_t i = C<Component>::header->size - 1; i != 0u; --i) {
            func(C<Component>::get_by_handle(i));
        }
    }
};

}
