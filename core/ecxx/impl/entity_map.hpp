#pragma once

#include <type_traits>
#include "entity.hpp"
#include "sparse_vector.hpp"

namespace ecs {

class entity_map_base {
public:
    using index_type = entity::index_type;
    using entity_vector_type = std::vector<entity>;
    using entity_vector_iterator = std::vector<entity>::iterator;
    using entity_vector_const_iterator = std::vector<entity>::const_iterator;
    using entity_index_table = sparse_vector;

    entity_map_base() {
        entity_.emplace_back();
    }

    virtual ~entity_map_base() = default;

    virtual void emplace_dyn(entity) = 0;

    virtual void erase_dyn(index_type) = 0;

    virtual void clear() = 0;

    inline bool has(index_type idx) const {
        return table_.has(idx);
    }

    inline const entity_index_table& index_table() const {
        return table_;
    }

    inline entity_vector_iterator begin() const {
        return ++(const_cast<entity_vector_type&>(entity_).begin());
    }

    inline entity_vector_iterator end() const {
        return const_cast<entity_vector_type&>(entity_).end();
    }

    inline const entity& at(uint32_t index) const {
        return entity_[index];
    }

    inline const entity* at_pointer(uint32_t index) const {
        return entity_.data() + index;
    }

    inline index_type size() const {
        return static_cast<index_type>(entity_.size() - 1);
    }

    inline index_type vector_size() const {
        return static_cast<index_type>(entity_.size());
    }

    inline bool locked() const {
        return locks_ != 0u;
    }

    class locker final {
    public:

        locker() noexcept
                : counter_{nullptr} {
        }

        explicit locker(uint32_t& counter) noexcept
                : counter_{&counter} {
            ++(*counter_);
        }

        ~locker() noexcept {
            if (counter_) {
                --(*counter_);
            }
        }

        inline constexpr locker(locker&& e) noexcept {
            counter_ = e.counter_;
            e.counter_ = nullptr;
        }

        inline constexpr locker(const locker& e) noexcept {
            counter_ = e.counter_;
            if (counter_) {
                ++(*counter_);
            }
        }

        inline constexpr locker& operator=(locker&& e) noexcept {
            counter_ = e.counter_;
            e.counter_ = nullptr;
            return *this;
        }

        inline constexpr locker& operator=(const locker& e) noexcept {
            counter_ = e.counter_;
            if (counter_) {
                ++(*counter_);
            }
            return *this;
        }


    private:
        uint32_t* counter_{};
    };

    inline locker lock() const {
        return locker{locks_};
    }

protected:

    // entity index -> data slot
    entity_index_table table_;

    // entity vector
    entity_vector_type entity_;

    // lock counter
    mutable uint32_t locks_ = 0u;
};

template<typename DataType>
class entity_map final : public entity_map_base {
public:
    using base_type = entity_map_base;
    using index_type = entity::index_type;
    using data_type = DataType;

    static constexpr bool has_data = std::negation_v<typename std::is_empty<data_type>::type>;

    entity_map() {
        // null data
        data_.emplace_back();
    }

    ~entity_map() override = default;

    template<typename ...Args>
    DataType& emplace(entity e, Args&& ...args) {
        assert(!base_type::locked());
        assert(!base_type::has(e.index()));

        auto di = static_cast<index_type>(base_type::entity_.size());
        base_type::entity_.emplace_back(e);
        base_type::table_.insert(e.index(), di);

        if constexpr (has_data) {
            if constexpr (std::is_aggregate_v<data_type>) {
                return data_.emplace_back(data_type{args...});
            } else {
                return data_.emplace_back(args...);
            }
        } else {
            return data_[0u];
        }
    }

    void erase(index_type ei) {
        assert(!base_type::locked());
        assert(base_type::has(ei));

        const auto index = base_type::table_.get_and_remove(ei);
        const bool swap_with_back = index < base_type::entity_.size() - 1u;

        if (swap_with_back) {
            const entity back_entity = base_type::entity_.back();
            base_type::table_.replace(back_entity.index(), index);
            std::swap(base_type::entity_.back(), base_type::entity_[index]);

            if constexpr (has_data) {
                std::swap(data_.back(), data_[index]);
            }
        }
        base_type::entity_.pop_back();
        if constexpr (has_data) {
            data_.pop_back();
        }
    }

    inline DataType& get(index_type idx) const {
        assert(base_type::has(idx));
        if constexpr (has_data) {
            return get_data(base_type::table_.at(idx));
        } else {
            return get_data(0u);
        }
    }

    DataType& get_or_create(entity e) {
        const auto idx = e.index();
        if (!base_type::has(idx)) {
            emplace(e);
        }
        return get(idx);
    }

    DataType& get_or_default(index_type idx) const {
        if constexpr (has_data) {
            return get_data(base_type::has(idx) ? base_type::table_.at(idx) : 0u);
        } else {
            return get_data(0u);
        }
    }

    void emplace_dyn(entity e) final {
        emplace(e);
    }

    void erase_dyn(index_type idx) final {
        erase(idx);
    }

    void clear() final {
        entity_.clear();
        entity_.emplace_back();
        data_.clear();
        data_.emplace_back();
        table_.clear();
    }

    inline DataType& get_data(index_type i) const {
        return const_cast<DataType&>(data_[i]);
    }

    inline DataType& get_data_by_entity_index(index_type ei) const {
        if constexpr (has_data) {
            return get_data_by_index(table_.at(ei));
        } else {
            return const_cast<DataType&>(data_[0u]);
        }
    }

    // get data by packed index
    inline DataType& get_data_by_index(index_type i) const {
        if constexpr (has_data) {
            return const_cast<DataType&>(data_[i]);
        } else {
            return const_cast<DataType&>(data_[0u]);
        }
    }

private:
    std::vector<data_type> data_;
};

}
