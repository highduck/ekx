#pragma once

#include "world.hpp"
#include <ek/ds/SmallArray.hpp>

namespace ecs {

template<typename ...Component>
class view_backward_t {
public:
    static constexpr auto components_num = sizeof ... (Component);

    using table_index_type = uint32_t;
    using table_type = ComponentHeader*[components_num];

    class iterator final {
    public:
        iterator(table_type& table, Entity it) : it_{it},
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
        inline bool valid(Entity e) const {
            for (uint32_t i = 1u; i < components_num; ++i) {
                if (table_[i]->entityToHandle.get(e) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline entity operator*() const {
            return entity{ent_};
        }

        inline entity operator*() {
            return entity{ent_};
        }

    private:
        Entity it_;
        Entity ent_;
        const table_type& table_;
        const ComponentHeader* map_0;
    };

    explicit view_backward_t(world* w) {
        table_index_type i{};
        ((access_[i] = table_[i] = w->getComponentHeader(type<Component>()), ++i), ...);

        std::sort(table_, table_ + components_num, [](auto a, auto b) -> bool {
            return a->count() < b->count();
        });
    }

    iterator begin() {
        return {table_, static_cast<Entity>(table_[0]->count() - 1)};
    }

    iterator end() {
        return {table_, 0};
    }

    template<typename Comp>
    constexpr inline Comp& unsafe_get(table_index_type i, Entity idx) {
        return static_cast<ComponentStorage <Comp>*>(access_[i]->data)->get(idx);
    }

    template<typename Func>
    void each(Func func) {
        for (auto e : *this) {
            table_index_type i{0u};
            func(unsafe_get<Component>(i++, e.index)...);
        }
    }

private:
    table_type access_;
    table_type table_;
};

}
