#include "world.hpp"

#include <Tracy.hpp>

namespace ecs {

world the_world;

/** World **/

void resetEntityPoolArrays(Entity* entities, Generation* generations) {
    memset(generations, 0, sizeof(Generation) * ENTITIES_MAX_COUNT);
    {
        auto* it = entities;
        for (uint32_t i = 1; i <= ENTITIES_MAX_COUNT; ++i) {
            *(it++) = i;
        }
    }
}

void world::resetEntityPool() {
    resetEntityPoolArrays(entityPool, generations);

    // reserve null entity
    entityPool[0] = 0;
    next = 1;
    size = 1;
}

void world::create(Entity* outEntities, uint32_t count_) {
    auto* indices = entityPool;
    auto next_ = next;

    for (uint32_t i = 0; i < count_; ++i) {
        const auto nextIndex = indices[next_];
        indices[next_] = next_;
        outEntities[i] = next_;
        next_ = nextIndex;
    }
    next = next_;
    size += count_;
}

void world::destroy(const Entity* entitiesToDestroy, uint32_t count) {
    {
        // destroy from POOL
        auto* indices = entityPool;
        auto* generations_ = generations;
        auto next_ = next;
        for (uint32_t i = 0; i < count; ++i) {
            const auto index = entitiesToDestroy[i];
            indices[index] = next_;
            ++generations_[index];
            next_ = index;
        }
        next = next_;
        size -= count;
    }
    {
        // destroy components
        auto** components_ = components;
        for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
            auto* component = components_[i];
            // TODO: we can know all components ahead of time and register/init them and not check every case...
            if (component) {
                auto* erase = component->erase;
                const auto& entityToHandle = component->entityToHandle;
                for (uint32_t j = 0; j < count; ++j) {
                    const auto entity = entitiesToDestroy[j];
                    const auto handle = entityToHandle.get(entity);
                    if (handle != 0) {
                        erase(component, entity);
                    }
                }
            }
        }
    }
}

bool world::check(Passport passport) const {
    const auto index = passport & INDEX_MASK;
    const auto generation = (passport >> INDEX_BITS) & GENERATION_MASK;
    return generation == generations[index];
}

// World create / destroy

void world::initialize() {
    resetEntityPool();
    memset(components, 0, COMPONENTS_MAX_COUNT * sizeof(void*));
}

void world::reset() {
    resetEntityPool();
    auto** components_ = components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components_[i];
        if (component) {
            component->clear(component);
            component->entityToHandle.clear();
            component->handleToEntity.size = 1;
        }
    }
}

void world::shutdown() {
    // skip clearing entity pool, because we don't need it anymore
    auto** components_ = components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components_[i];
        if (component) {
            component->entityToHandle.clear();
            delete component;
        }
    }
}

void component_type_init(ComponentHeader* component, ComponentTypeId typeId, void* userData) {
    component->typeId = typeId;
    component->lockCounter = 0;
    component->data = userData;
    component->emplace = nullptr;
    component->erase = nullptr;
    component->clear = nullptr;
    component->entityToHandle.init();
    component->handleToEntity.push_back(0);
}

}
