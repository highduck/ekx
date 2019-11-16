#pragma once

#include "entity_map.hpp"
#include "identity_generator.hpp"

namespace ecxx {

class components_db final {
public:
    using component_typeid = spec::component_typeid;

    ~components_db() {
        for (auto* pool : pools_) {
            delete pool;
        }
    }

    template<typename Component>
    inline static constexpr component_typeid type() noexcept {
        return identity_generator<Component, component_typeid>::value;
    }

    template<typename Component>
    entity_map<Component>& ensure() {
        const auto cid = type<Component>();
        if (cid < pools_.size()) {
        } else {
            pools_.resize(cid + 1u);
        }

        auto* map = pools_[cid];
        if (map != nullptr) {
        } else {
            map = new entity_map<Component>();
            pools_[cid] = map;
        }
        return *static_cast<entity_map<Component>*>(map);
    }

    template<typename Component>
    inline const entity_map<Component>* try_get() const {
        return static_cast<const entity_map<Component>*>(try_get(type<Component>()));
    }

    template<typename Component>
    inline entity_map<Component>* try_get() {
        return static_cast<entity_map<Component>*>(try_get(type<Component>()));
    }

    inline entity_map_base* try_get(component_typeid type) {
        return type < pools_.size() ? pools_[type] : nullptr;
    }

    [[nodiscard]] inline const entity_map_base* try_get(component_typeid type) const {
        return type < pools_.size() ? pools_[type] : nullptr;
    }

    void remove_all_c(entity e) {
        for (auto* pool : pools_) {
            if (pool != nullptr && pool->has(e)) {
                pool->erase_dyn(e);
            }
        }
    }

private:
    std::vector<entity_map_base*> pools_;
};

}