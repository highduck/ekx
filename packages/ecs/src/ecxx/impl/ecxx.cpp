#include "world.hpp"
#include <ek/log.h>
#include <ek/assert.h>
#include <cstring>

namespace ecs {

World the_world;

/** World **/

void resetEntityPoolArrays(EntityIndex* entities, EntityGeneration* generations) {
    memset(generations, 0, sizeof(EntityGeneration) * ENTITIES_MAX_COUNT);
    {
        auto* it = entities;
        for (uint32_t i = 1; i <= ENTITIES_MAX_COUNT; ++i) {
            *(it++) = i;
        }
    }
}

void World::resetEntityPool() {
    resetEntityPoolArrays(entityPool, generations);

    // reserve null entity
    entityPool[0] = 0;
    next = 1;
    size = 1;
}

void World::create(EntityIndex* outEntities, uint32_t count_) {
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

void World::destroy(const EntityIndex* entitiesToDestroy, uint32_t count) {
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
                auto* run = component->run;
                const auto& entityToHandle = component->entityToHandle;
                for (uint32_t j = 0; j < count; ++j) {
                    const auto entity = entitiesToDestroy[j];
                    const auto handle = entityToHandle.get(entity);
                    if (handle != 0) {
                        run(StorageCommand::Erase, component, entity);
                    }
                }
            }
        }
    }
}

bool World::check(EntityPassport passport) const {
    const auto index = passport & INDEX_MASK;
    const auto generation = (passport >> INDEX_BITS) & GENERATION_MASK;
    return generation == generations[index];
}

// World create / destroy

void World::initialize() {
    EK_DEBUG("ecs::world initialize");
    resetEntityPool();
    if(EntityLookup::pInvalidPage == nullptr) {
        EntityLookup::pInvalidPage = (EntityLookup::Page*)malloc(EntityLookup::PageSize);
        memset(EntityLookup::pInvalidPage, 0, EntityLookup::PageSize);
    }
    memset(components, 0, COMPONENTS_MAX_COUNT * sizeof(void*));
}

void World::reset() {
    resetEntityPool();
    auto** components_ = components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components_[i];
        if (component) {
            component->run(StorageCommand::Clear, component, 0);
            component->entityToHandle.clear();
            component->handleToEntity.reduceSize(1);
        }
    }
}

void World::shutdown() {
    EK_DEBUG("ecs::world shutdown");
    // skip clearing entity pool, because we don't need it anymore
    auto** components_ = components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components_[i];
        if (component) {
            component->entityToHandle.clear();
            component->run(StorageCommand::Shutdown, component, 0);
            free(component->data);
        }
    }
}

}
