#pragma once

#include "world.hpp"
#include <array>

namespace ecs {

template<typename ...Component>
class view_backward_t {
public:
    static constexpr auto components_num = sizeof ... (Component);

    using table_type = std::array<entity_map_base*, components_num>;
    using table_index_type = uint32_t;

    using indices_type = std::array<table_index_type, components_num>;

    using entity_vector_iterator = typename std::vector<entity>::iterator;

    class iterator final {
    public:
        iterator(table_type& table, Entity it) : it_{it},
                                                 table_{table},
                                                 map_0{*table[0]} {
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
            ECXX_ASSERT(it_ < map_0.count);

            while (it_ != 0 && !valid(map_0.handleToEntity[it_])) {
                --it_;
            }
            ent_ = map_0.handleToEntity[it_];
        }

        [[nodiscard]]
        inline bool valid(Entity e) const {
            for (uint32_t i = 1u; i < components_num; ++i) {
                if (sparse_array_get(table_[i]->entityToHandle, e) == 0) {
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
        const entity_map_base& map_0;
    };

    explicit view_backward_t(world* db) {
        table_index_type i{};
        ((access_[i] = table_[i] = &component_ensure<Component>(db), ++i), ...);

        std::sort(table_.begin(), table_.end(), [](auto a, auto b) -> bool {
            return a->count < b->count;
        });
    }

    iterator begin() {
        return {table_, static_cast<Entity>(table_[0]->count - 1)};
    }

    iterator end() {
        return {table_, 0};
    }

    template<typename Comp>
    constexpr inline Comp& unsafe_get(table_index_type i, Entity idx) {
        return static_cast<entity_map <Comp>*>(access_[i])->get(idx);
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