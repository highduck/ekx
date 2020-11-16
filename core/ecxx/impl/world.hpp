#pragma once

#include "entity_pool.hpp"
#include "view.hpp"
#include "runtime_view.hpp"
#include "rview.hpp"

namespace ecs {

class world final {
private:
    world() = default;

public:

    world(const world&) = delete;

    inline void reserve(size_t size) {
        entities.reserve(size);
    }

    template<typename ...Component>
    inline entity create() {
        auto e = entities.allocate();
        (assign<Component>(e), ...);
        return e;
    }

    template<typename ...Component, typename It>
    void create(It begin, It end) {
        entities.allocate(begin, end);
        if constexpr (sizeof...(Component) > 0) {
            for (auto it = begin; it != end; ++it) {
                (assign<Component>(*it), ...);
            }
        }
    }

    inline void destroy(entity e) {
        components.remove_all_c(e.index());
        entities.deallocate(e);
    }

    template<typename Component, typename ...Args>
    inline Component& assign(entity e, Args&& ... args) {
        auto& pool = components.ensure<Component>();
        return pool.emplace(e, args...);
    }

    template<typename Component, typename ...Args>
    inline Component& reassign(entity e, Args&& ... args) {
        auto& pool = components.ensure<Component>();
        const auto idx = e.index();
        if (pool.has(idx)) {
            auto& data = pool.get(idx);
            data = {args...};
            return data;
        }
        return pool.emplace(e, args...);
    }

    template<typename Component>
    [[nodiscard]] inline bool has(entity::index_type idx) const {
        const auto* pool = components.try_get<Component>();
        return pool && pool->has(idx);
    }

    template<typename Component>
    inline const Component& get(entity::index_type idx) const {
        const auto* pool = components.try_get<Component>();
        assert(pool);
        return pool->get(idx);
    }

    template<typename Component>
    inline Component* tryGet(entity::index_type idx) {
        auto* pool = components.try_get<Component>();
        return pool ? pool->tryGet(idx) : nullptr;
    }

    template<typename Component>
    inline Component& get(entity::index_type idx) {
        auto* pool = components.try_get<Component>();
        assert(pool);
        return pool->get(idx);
    }

    template<typename Component>
    inline Component& get_or_create(entity e) {
        auto& pool = components.ensure<Component>();
        return pool.get_or_create(e);
    }

    template<typename Component>
    inline const Component& get_or_default(entity::index_type idx) const {
        const auto& pool = const_cast<components_db&>(components).ensure<Component>();
        return pool.get_or_default(idx);
    }

    template<typename Component>
    inline void remove(entity::index_type idx) {
        auto* pool = components.try_get<Component>();
        assert(pool);
        pool->erase(idx);
    }

    template<typename Component>
    inline bool try_remove(entity::index_type idx) {
        auto* pool = components.try_get<Component>();
        if (pool && pool->has(idx)) {
            pool->erase(idx);
            return true;
        }
        return false;
    }

    template<typename Component>
    [[nodiscard]] inline bool is_locked() const {
        const auto* pool = components.try_get<Component>();
        return pool && pool->locked();
    }

    template<typename ...Component>
    inline auto view() {
        return basic_view<Component...>{components};
    }

    /** special view provide back-to-front iteration
        and allow modify primary component map during iteration
     **/
    template<typename ...Component>
    inline auto rview() {
        return basic_rview<Component...>{components};
    }

    template<typename It>
    inline runtime_view_t runtime_view(It begin, It end) {
        std::vector<entity_map_base*> table;
        for (auto it = begin; it != end; ++it) {
            auto* set = components.try_get(*it);
            if (set != nullptr) {
                table.emplace_back(set);
            }
        }
        return runtime_view_t(table);
    }

    template<typename Component>
    constexpr inline unsigned type() noexcept {
        return details::identity_generator<Component>::value;
    }

    [[nodiscard]] inline bool valid(entity e) const {
        return e && entities.current(e.index()) == e.version();
    }

    inline auto& components_data() {
        return components;
    }

    void clear() {
        components.clear();
        entities.clear();
    }

public:
    basic_entity_pool entities;
    components_db components;

public:
    static world the;
};

inline world world::the{};

}