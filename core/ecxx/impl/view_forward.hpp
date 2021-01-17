#pragma once

#include "world.hpp"
#include <array>

namespace ecs {

template<typename ...Component>
class view_forward_t {
public:
    static constexpr auto components_num = sizeof ... (Component);

    using table_type = std::array<entity_map_base*, components_num>;
    using table_index_type = uint32_t;

    using indices_type = std::array<table_index_type, components_num>;

    using entity_vector_iterator = typename std::vector<entity>::iterator;

    class iterator final {
    public:
        iterator(const table_type& table, uint32_t it) noexcept: it_{it},
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
            if (it == m.count) {
                return true;
            }
            // filter secondary entity vectors
            const auto entity = m.handleToEntity[it];
            for (uint32_t k = 1u; k < components_num; ++k) {
                if (sparse_array_get(table[k]->entityToHandle, entity) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline static bool is_valid_fast(Entity idx, const table_type& table) {
            // filter secondary entity vectors
            const uint32_t cn = components_num;
            for (uint32_t k = 1u; k < cn; ++k) {
                if (sparse_array_get(table[k]->entityToHandle, idx) == 0) {
                    return false;
                }
            }
            return true;
        }

        inline entity operator*() const noexcept {
            return entity{table_[0]->handleToEntity[it_]};
        }

        inline entity operator*() noexcept {
            return entity{table_[0]->handleToEntity[it_]};
        }

    private:
        uint32_t it_{};
        const table_type& table_;
    };

    explicit view_forward_t(world* w) {
        {
            table_index_type i{};
            ((access_[i] = table_[i] = &component_ensure<Component>(w), ++i), ...);
        }

        std::sort(table_.begin(), table_.end(), [](auto* a, auto* b) -> bool {
            return a->count < b->count;
        });

        for (uint32_t j = 0u; j < components_num; ++j) {
            ++access_[j]->lock_counter;
        }
    }

    ~view_forward_t() {
        for (uint32_t j = 0u; j < components_num; ++j) {
            --access_[j]->lock_counter;
        }
    }

    iterator begin() const {
        return {table_, 1u};
    }

    iterator end() const {
        return {table_, table_[0]->count};
    }

    template<typename Comp>
    constexpr inline Comp& unsafe_get(table_index_type i, Entity ei) const {
        return static_cast<const entity_map <Comp>*>(access_[i])->get(ei);
    }

    template<typename Func>
    void each(Func func) const {
        const entity_map_base& table_0 = *(table_[0]);
        for (uint32_t i = 1u; i != table_0.count; ++i) {
            const Entity e = table_0.handleToEntity[i];
            if (iterator::is_valid_fast(e, table_)) {
                table_index_type k{0u};
                func(unsafe_get<Component>(k++, e)...);
            }
        }
    }

private:
    table_type access_;
    table_type table_;
};

template<typename Component>
class view_forward_t<Component> {
public:
    static constexpr auto components_num = 1;

    class iterator final {
    public:
        iterator(const entity_map <Component>& m, uint32_t it) noexcept
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

        inline entity operator*() const noexcept {
            return entity{map_.handleToEntity[it_]};
        }

        inline entity operator*() noexcept {
            return entity{map_.handleToEntity[it_]};
        }

    private:
        uint32_t it_{};
        const entity_map <Component>& map_;
    };

    explicit view_forward_t(world* w) :
            map_{component_ensure<Component>(w)} {
        ++map_.lock_counter;
    }

    ~view_forward_t() {
        --map_.lock_counter;
    }

    iterator begin() const {
        return {map_, 1u};
    }

    iterator end() const {
        return {map_, map_.count};
    }

    template<typename Func>
    void each(Func func) const {
        for (uint32_t i = 1u; i != map_.count; ++i) {
            func(map_.get_data_by_index(i));
        }
    }

private:
    entity_map <Component>& map_;
};

}