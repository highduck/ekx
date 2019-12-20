#pragma once

#include "entity_pool.hpp"
#include "view.hpp"
#include "runtime_view.hpp"
#include "rview.hpp"

namespace ecxx {

class world final {
public:
    using component_typeid = spec::component_typeid;

    world() = default;

    world(const world&) = delete;

    inline void reserve(size_t size) {
        pool_.reserve(size);
    }

    template<typename ...Component>
    inline entity create() {
        auto e = pool_.allocate();
        (assign<Component>(e), ...);
        return e;
    }

    template<typename ...Component, typename It>
    void create(It begin, It end) {
        pool_.allocate(begin, end);
        if constexpr (sizeof...(Component) > 0) {
            for (auto it = begin; it != end; ++it) {
                (assign<Component>(*it), ...);
            }
        }
    }

    inline void destroy(entity e) {
        components_.remove_all_c(e);
        pool_.deallocate(e);
    }

    template<typename Func>
    inline void each(Func func) const {
        pool_.each(func);
    }

    template<typename Component, typename ...Args>
    inline Component& assign(entity e, Args&& ... args) {
        auto& pool = components_.ensure<Component>();
        return pool.emplace(e, args...);
    }

    template<typename Component, typename ...Args>
    inline Component& replace_or_assign(entity e, Args&& ... args) {
        auto& pool = components_.ensure<Component>();
        if (pool.has(e)) {
            auto& data = pool.get(e);
            data = {args...};
            return data;
        }
        return pool.emplace(e, args...);
    }

    template<typename Component>
    [[nodiscard]] inline bool has(entity e) const {
        const auto* pool = components_.try_get<Component>();
        return pool && pool->has(e);
    }

    template<typename Component>
    inline const Component& get(entity e) const {
        const auto* pool = components_.try_get<Component>();
        assert(pool);
        return pool->get(e);
    }

    template<typename Component>
    inline Component& get(entity e) {
        auto* pool = components_.try_get<Component>();
        assert(pool);
        return pool->get(e);
    }

    template<typename Component>
    inline Component& get_or_create(entity e) {
        auto& pool = components_.ensure<Component>();
        return pool.get_or_create(e);
    }

    template<typename Component>
    inline const Component& get_or_default(entity e) const {
        const auto& pool = const_cast<components_db&>(components_).ensure<Component>();
        return pool.get_or_default(e);
    }

    template<typename Component>
    inline void remove(entity e) {
        auto* pool = components_.try_get<Component>();
        assert(pool);
        pool->erase(e);
    }

    template<typename Component>
    inline bool try_remove(entity e) {
        auto* pool = components_.try_get<Component>();
        if (pool) {
            if (pool->has(e)) {
                pool->erase(e);
                return true;
            }
        }
        return false;
    }

    template<typename Component>
    [[nodiscard]] inline bool is_locked() const {
        const auto* pool = components_.try_get<Component>();
        return pool && pool->locked();
    }

    template<typename ...Component>
    inline auto view() {
        return basic_view<Component...>{components_};
    }

    /** special view provide back-to-front iteration
        and allow modify primary component map during iteration
     **/
    template<typename ...Component>
    inline auto rview() {
        return basic_rview<Component...>{components_};
    }

    template<typename It>
    inline runtime_view_t runtime_view(It begin, It end) {
        std::vector<entity_map_base*> table;
        for (auto it = begin; it != end; ++it) {
            auto* set = components_.try_get(*it);
            if (set != nullptr) {
                table.emplace_back(set);
            }
        }
        return runtime_view_t(table);
    }

    [[nodiscard]] inline const auto& pool() const {
        return pool_;
    }

    inline auto& pool() {
        return pool_;
    }

    template<typename Component>
    constexpr inline component_typeid type() noexcept {
        return identity_generator<Component, component_typeid>::value;
    }

    [[nodiscard]] inline bool valid(entity e) const {
        return e && pool_.current(e.index()) == e.version();
    }

    inline auto& components_data() {
        return components_;
    }

private:
    basic_entity_pool pool_;
    components_db components_;
};

}