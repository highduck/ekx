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
        iterator(table_type& table, EntityIndex it) : it_{it},
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
            EK_ASSERT_R2(it_ < map_0->count());

            while (it_ != 0 && !valid(map_0->handleToEntity.get(it_))) {
                --it_;
            }
            ent_ = map_0->handleToEntity.get(it_);
        }

        [[nodiscard]]
        inline bool valid(EntityIndex e) const {
            for (uint32_t i = 1u; i < components_num; ++i) {
                if (table_[i]->entityToHandle.get(e) == 0) {
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
        EntityIndex it_;
        EntityIndex ent_;
        const table_type& table_;
        const ComponentHeader* map_0;
    };

    explicit ViewBackward(World& w) {
        table_index_type i{};
        ((access_[i] = table_[i] = w.getComponentHeader(type<Component>()), ++i), ...);
        qsort(table_, components_num, sizeof(table_[0]), ComponentHeader::compareBySize);
    }

    iterator begin() {
        return {table_, static_cast<EntityIndex>(table_[0]->count() - 1)};
    }

    iterator end() {
        return {table_, 0};
    }

    template<typename Comp>
    constexpr inline Comp& unsafe_get(table_index_type i, EntityIndex idx) {
        return static_cast<ComponentStorage <Comp>*>(access_[i]->data)->get(idx);
    }

    template<typename Func>
    void each(Func func) {
        for (auto e: *this) {
            table_index_type i{0u};
            func(unsafe_get<Component>(i++, e.index)...);
        }
    }

private:
    table_type access_;
    table_type table_;
};

}
