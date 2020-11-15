#pragma once

#include "entity.hpp"

#include <vector>

namespace ecs {

class basic_entity_pool {
public:
    using value_type = entity;
    using index_type = entity::index_type;
    using version_type = entity::version_type;

    basic_entity_pool() {
        list_.emplace_back();
    }

    inline void clear() {
        list_.clear();
        list_.emplace_back();
        next_ = 0u;
        available_ = 0u;
    }

    inline value_type allocate() {
        if (available_) {
            value_type node = list_[next_];
            value_type e = value_type{next_, node.version()};
            list_[next_] = e;
            next_ = node.index();
            --available_;
            return e;
        }
        // construct new from value type, with 0 version
        return list_.emplace_back(static_cast<index_type>(list_.size()));
    }

    template<typename It>
    void allocate(It begin, const It end) {
        while (begin != end) {
            // TODO: batch
            *begin = allocate();
            ++begin;
        }
    }

    void deallocate(value_type entity) {
        const auto i = entity.index();
        list_[i] = {next_, static_cast<version_type>(entity.version() + 1u)};
        next_ = i;
        ++available_;
    }

    inline void reserve(size_t size) {
        list_.reserve(size + 1);
    }

    [[nodiscard]] inline index_type size() const {
        return static_cast<index_type>(list_.size() - available_ - 1u);
    }

    /*
     * query current version for entity index
     * index could be:
     *  - alive (index match #slot)
     *  - dead (index doesn't match #slot)
     *  - null
     */
    [[nodiscard]] inline version_type current(index_type idx) const {
        //assert(is_alive(i));
        return list_[idx].version();
    }

    [[nodiscard]] inline bool indexIsAlive(index_type idx) const {
        return idx < list_.size() && idx == list_[idx].index();
    }

    [[nodiscard]] inline index_type available_for_recycling() const {
        return available_;
    }

    template<typename Func>
    void each(Func func) const {
        static_assert(std::is_invocable_v<Func, value_type>);

        const auto end = static_cast<index_type>(list_.size());

        if (available_) {
            for (index_type i = 1u; i != end; ++i) {
                const value_type e = list_[i];
                if (e.index() == i) {
                    func(e);
                }
            }
        } else {
            for (index_type i = 1u; i != end; ++i) {
                func(list_[i]);
            }
        }
    }

private:
    std::vector<value_type> list_;
    index_type next_{0u};
    index_type available_{0u};
};

}