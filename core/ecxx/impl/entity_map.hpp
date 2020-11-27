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
        entities.emplace_back();
    }

    virtual ~entity_map_base() = default;

    virtual void emplace_dyn(entity) = 0;

    virtual void erase_dyn(index_type) = 0;

    virtual void clear() = 0;

    inline entity_vector_iterator begin() const {
        return ++(const_cast<entity_vector_type&>(entities).begin());
    }

    inline entity_vector_iterator end() const {
        return const_cast<entity_vector_type&>(entities).end();
    }

    inline const entity& at(uint32_t index) const {
        return entities[index];
    }

    inline const entity* at_pointer(uint32_t index) const {
        return entities.data() + index;
    }

    inline index_type size() const {
        return static_cast<index_type>(entities.size() - 1);
    }

    inline index_type vector_size() const {
        return static_cast<index_type>(entities.size());
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

    // entity index -> data slot
    entity_index_table dataTable;

    // entity vector
    entity_vector_type entities;

protected:

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
        bool locked_debug = locked();
        ECXX_ASSERT(locks_ == 0);
        ECXX_ASSERT(!dataTable.has(e.index()));

        auto di = static_cast<index_type>(entities.size());
        dataTable.insert(e.index(), di);
        entities.push_back(e);

        if constexpr (has_data) {
            if constexpr (std::is_aggregate_v<data_type>) {
//                return data_.emplace_back(data_type{std::forward<Args>(args)...});
                return data_.emplace_back(data_type{args...});
            } else {
//                return data_.emplace_back(data_type(std::forward<Args>(args)...));
                return data_.emplace_back(args...);
            }
        } else {
            return data_[0u];
        }
    }

    void erase(index_type ei) {
        ECXX_ASSERT(!base_type::locked());
        ECXX_ASSERT(base_type::dataTable.has(ei));

        const auto index = base_type::dataTable.get_and_remove(ei);
        const bool swap_with_back = index < base_type::entities.size() - 1u;

        if (swap_with_back) {
            const entity back_entity = base_type::entities.back();
            base_type::dataTable.replace(back_entity.index(), index);
            std::swap(base_type::entities.back(), base_type::entities[index]);

            if constexpr (has_data) {
                std::swap(data_.back(), data_[index]);
            }
        }
        base_type::entities.pop_back();
        if constexpr (has_data) {
            data_.pop_back();
        }
    }

    inline DataType& get(index_type idx) const {
        if constexpr (has_data) {
            return get_data(base_type::dataTable.at(idx));
        } else {
            return get_data(0u);
        }
    }

    inline DataType* tryGet(index_type idx) {
        DataType* ptr = nullptr;
        if (base_type::dataTable.has(idx)) {
            if constexpr (has_data) {
                ptr = data_.data() + base_type::dataTable.at(idx);
            } else {
                ptr = data_.data();
            }
        }
        return ptr;
    }

    DataType& get_or_create(entity e) {
        const auto idx = e.index();
        if (!base_type::dataTable.has(idx)) {
            emplace(e);
        }
        return get(idx);
    }

    DataType& get_or_default(index_type idx) const {
        if constexpr (has_data) {
            return get_data(base_type::dataTable.has(idx) ? base_type::dataTable.at(idx) : 0u);
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
        entities.clear();
        entities.emplace_back();
        data_.clear();
        data_.emplace_back();
        dataTable.clear();
    }

    inline DataType& get_data(index_type i) const {
        return const_cast<DataType&>(data_[i]);
    }

    inline DataType& get_data_by_entity_index(index_type ei) const {
        if constexpr (has_data) {
            return get_data_by_index(dataTable.at(ei));
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
