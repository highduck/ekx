#pragma once

#include "world.hpp"
#include <ek/ds/FixedArray.hpp>
#include <cstdlib>

namespace ecs {

template<typename ...Component>
class ViewBackward {
public:
    static constexpr auto components_num = sizeof ... (Component);

    using table_index_type = uint32_t;
    using table_type = ComponentHeader* [components_num];

    class iterator final {
    public:
        iterator(table_type& table, entity_t it) : it_{it},
                                                      table_{table},
                                                      map_0{table[0]} {
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
            EK_ASSERT_R2(it_ < map_0->handleToEntity.size());

            while (it_ != 0 && !valid(map_0->handleToEntity.get(it_))) {
                --it_;
            }
            ent_ = map_0->handleToEntity.get(it_);
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
        const table_type& table_;
        const ComponentHeader* map_0;
    };

    ViewBackward() {
        {
            table_index_type i{};
            ((table_[i] = &C<Component>::header, ++i), ...);
        }
        qsort(table_, components_num, sizeof(table_[0]), ComponentHeader::compareBySize);
    }

    iterator begin() {
        return {table_, (entity_t)(table_[0]->handleToEntity.size() - 1)};
    }

    iterator end() {
        return {table_, 0};
    }

    template<typename Func>
    void each(Func func) {
#pragma nounroll
        for (auto e: *this) {
            func(*C<Component>::get_by_entity(e.index)...);
        }
    }

private:
    table_type table_;
};

}
