#pragma once

#include "world.hpp"
#include <cstdlib>

namespace ecs {

template<typename ...Cn>
class ViewBackward {
public:
    static constexpr auto components_num = sizeof ... (Cn);

    class iterator final {
    public:
        iterator(const ecx_component_type* const table[components_num], component_handle_t it) noexcept: it_{it},
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
        inline bool valid(entity_idx_t entity_idx) const {
            for (uint32_t i = 1u; i < components_num; ++i) {
                if (get_component_handle_by_index(table_[i], entity_idx) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline Entity operator*() const {
            return Entity{entity_at(ent_)};
        }

        inline Entity operator*() {
            return Entity{entity_at(ent_)};
        }

    private:
        component_handle_t it_;
        entity_idx_t ent_;
        const ecx_component_type* const* table_;
    };

    ViewBackward() noexcept {
        uint32_t i = 0;
        ((table_[i] = type<Cn>(), ++i), ...);
        _sort_component_type_table(table_, components_num);
    }

    iterator begin() const {
        return {table_, (component_handle_t) (table_[0]->size - 1)};
    }

    iterator end() const {
        return {table_, 0};
    }

    ecx_component_type* table_[components_num];
};

template<typename C>
class ViewBackward<C> {
public:
    static constexpr auto components_num = 1;

    class iterator final {
    public:
        iterator(component_handle_t it) noexcept: it_{it} {
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

        inline Entity operator*() const noexcept {
            return Entity{entity_at(type<C>()->handleToEntity[it_])};
        }

    private:
        component_handle_t it_ = 0;
    };

    ViewBackward() noexcept = default;

    iterator begin() const {
        const component_handle_t sz = type<C>()->size;
        return {(component_handle_t)(sz > 0 ? (sz - 1u) : 0u)};
    }

    iterator end() const {
        return {0u};
    }
};

}
