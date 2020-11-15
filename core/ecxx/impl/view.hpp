#pragma once

#include <array>

#include "components_db.hpp"

namespace ecs {

template<typename ...Component>
class basic_view {
public:
    using index_type = entity::index_type;
    static constexpr auto components_num = sizeof ... (Component);

    using table_type = std::array<const entity_map_base*, components_num>;
    using table_index_type = uint32_t;

    using indices_type = std::array<table_index_type, components_num>;

    using entity_vector_iterator = typename std::vector<entity>::iterator;

    class iterator final {
    public:
        iterator(const table_type& table, uint32_t it) noexcept
                : it_{it},
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
            const entity_map_base& m = *table[0];
            if (it == m.vector_size()) {
                return true;
            }
            // filter secondary entity vectors
            const auto idx = m.at(it).index();
            for (uint32_t k = 1u; k < components_num; ++k) {
                if (!table[k]->has(idx)) {
                    return false;
                }
            }
            return true;
        }

        inline static bool is_valid_fast(const index_type idx, const table_type& table) {
            // filter secondary entity vectors
            const uint32_t cn = components_num;
            for (uint32_t k = 1u; k < cn; ++k) {
                if (!table[k]->index_table().has(idx)) {
                    return false;
                }
            }
            return true;
        }

        inline const entity& operator*() const noexcept {
            return table_[0]->at(it_);
        }

        inline const entity* operator->() const noexcept {
            return table_[0]->at_pointer(it_);
        }

    private:
        uint32_t it_{};
        const table_type& table_;
    };

    explicit basic_view(components_db& db) {
        {
            table_index_type i{};
            ((access_[i] = table_[i] = &db.template ensure<Component>(), ++i), ...);
        }

        std::sort(table_.begin(), table_.end(), [](auto a, auto b) -> bool {
            return a->vector_size() < b->vector_size();
        });

        for (uint32_t j = 0u; j < components_num; ++j) {
            lockers_[j] = access_[j]->lock();
        }
    }

    iterator begin() const {
        return {table_, 1u};
    }

    iterator end() const {
        return {table_, table_[0]->vector_size()};
    }

    template<typename Comp>
    constexpr inline Comp& unsafe_get(table_index_type i, index_type ei) const {
        return static_cast<const entity_map<Comp>*>(access_[i])->get_data_by_entity_index(ei);
    }

    template<typename Func>
    void each(Func func) const {
        const entity_map_base& table_0 = *(table_[0]);
        for (uint32_t i = 1u; i != table_0.vector_size(); ++i) {
            const index_type ei = table_0.at(i).index();
            if (iterator::is_valid_fast(ei, table_)) {
                table_index_type k{0u};
                func(unsafe_get<Component>(k++, ei)...);
            }
        }
    }

private:
    table_type access_;
    table_type table_;
    std::array<entity_map_base::locker, components_num> lockers_;
};

template<typename Component>
class basic_view<Component> {
public:
    using index_type = entity::index_type;
    static constexpr auto components_num = 1;

    class iterator final {
    public:
        iterator(const entity_map<Component>& m, uint32_t it) noexcept
                : it_{it},
                  map_{m} {
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

        inline const entity& operator*() const noexcept {
            return map_.at(it_);
        }

        inline const entity* operator->() const noexcept {
            return map_.at_pointer(it_);
        }

    private:
        uint32_t it_{};
        const entity_map<Component>& map_;
    };

    explicit basic_view(components_db& db) :
            map_{db.template ensure<Component>()},
            locker_{map_.lock()} {
    }

    iterator begin() const {
        return {map_, 1u};
    }

    iterator end() const {
        return {map_, map_.vector_size()};
    }

    template<typename Func>
    void each(Func func) const {
        for (uint32_t i = 1u; i != map_.vector_size(); ++i) {
            func(map_.get_data_by_index(i));
        }
    }

private:
    const entity_map<Component>& map_;
    entity_map_base::locker locker_;
};

}