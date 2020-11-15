#pragma once

#include "entity_map.hpp"
#include "utility.hpp"

namespace ecs {

class components_db final {
public:
    using component_typeid = uint32_t;

    ~components_db() {
        for (auto* pool : pools_) {
            delete pool;
        }
    }

    void clear() {
        for (auto* pool : pools_) {
            if(pool) {
                pool->clear();
            }
        }
    }

    template<typename Component>
    inline static constexpr component_typeid type() noexcept {
        return details::identity_generator<Component>::value;
    }

    template<typename Component>
    entity_map<Component>& ensure() {
        const auto cid = type<Component>();
        if (cid < pools_.size()) {
        } else {
            //printf("resize pool because %d %s\n", cid, std::string{details::type_name<Component>()}.c_str());
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

    void remove_all_c(entity::index_type idx) {
        for (auto* pool : pools_) {
            if (pool != nullptr && pool->has(idx)) {
                pool->erase_dyn(idx);
            }
        }
    }

private:
    std::vector<entity_map_base*> pools_;
};

}