#pragma once

#include "components_db.hpp"

#include <array>

namespace ecxx {

template<typename ...Component>
class basic_rview {
public:
    static constexpr auto components_num = sizeof ... (Component);

    using table_type = std::array<entity_map_base*, components_num>;
    using table_index_type = uint32_t;

    using indices_type = std::array<table_index_type, components_num>;

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
            assert(it_ < map_0.vector_size());

            while (it_ != 0 && !valid(map_0.at(it_))) {
                --it_;
            }
            ent_ = map_0.at(it_);
        }

        [[nodiscard]] inline bool valid(entity e) const {
            for (uint32_t i = 1u; i < components_num; ++i) {
                if (!table_[i]->has(e)) {
                    return false;
                }
            }
            return true;
        }

        inline entity operator*() const {
            return ent_;
        }

        inline entity operator*() {
            return ent_;
        }

    private:
        uint32_t it_;
        entity ent_;
        const table_type& table_;
        const entity_map_base& map_0;
    };

    explicit basic_rview(components_db& db) {
        table_index_type i{};
        ((access_[i] = table_[i] = &db.template ensure<Component>(), ++i), ...);

        std::sort(table_.begin(), table_.end(), [](auto a, auto b) -> bool {
            return a->size() < b->size();
        });
    }

    iterator begin() {
        return {table_, table_[0]->vector_size() - 1};
    }

    iterator end() {
        return {table_, 0};
    }

    template<typename Comp>
    constexpr inline Comp& unsafe_get(table_index_type i, entity e) {
        return static_cast<entity_map <Comp>*>(access_[i])->get(e);
    }

    template<typename Func>
    void each(Func func) {
        for (auto e : *this) {
            table_index_type i{0u};
            func(unsafe_get<Component>(i++, e)...);
        }
    }

private:
    table_type access_;
    table_type table_;
};

}