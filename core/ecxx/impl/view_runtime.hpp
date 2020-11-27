#pragma once

#include "entity_map.hpp"

namespace ecs {

class view_runtime_t {
public:
    using table_type = std::vector<entity_map_base*>;
    using table_index_type = typename table_type::size_type;

    using indices_type = std::vector<table_index_type>;

    using entity_vector_iterator = typename std::vector<entity>::iterator;

    class iterator {
    public:
        iterator(table_type& table, uint32_t it)
                : it_{it},
                  table_{table},
                  map_0{*table[0]} {
            skips();
        }

        inline iterator& operator++() noexcept {
            if (it_ < map_0.vector_size() && map_0.at(it_) == ent_) {
                ++it_;
            }
            skips();
            return *this;
        }

        inline bool operator!=(const iterator& other) const noexcept {
            return ent_ != other.ent_;
        }

        inline void skips() {
            const auto size = map_0.vector_size();
            while (it_ < size && !valid(map_0.at(it_))) {
                ++it_;
            }
            ent_ = it_ < size ? map_0.at(it_) : entity{};
        }

        [[nodiscard]] inline bool valid(entity e) const {
            const auto end = static_cast<uint32_t>(table_.size());
            const auto idx = e.index();
            for (uint32_t i = 1u; i < end; ++i) {
                if (!table_[i]->dataTable.has(idx)) {
                    return false;
                }
            }
            return true;
        }

        inline entity operator*() const noexcept {
            return ent_;
        }

        inline entity operator*() noexcept {
            return ent_;
        }

    private:
        uint32_t it_;
        entity ent_;
        const table_type& table_;
        const entity_map_base& map_0;
    };

    explicit view_runtime_t(table_type& table)
            : access_{table},
              table_{table} {

        std::sort(table_.begin(), table_.end(), [](auto a, auto b) -> bool {
            return a->size() < b->size();
        });
    }

    iterator begin() {
        return {table_, 1u};
    }

    iterator end() {
        return {table_, table_[0]->vector_size()};
    }

    template<typename Func>
    void each(Func func) {
        for (auto e : *this) {
            func(e);
        }
    }

private:
    table_type access_;
    table_type table_;
};

}