#include "world.hpp"

#include <Tracy.hpp>

namespace ecs {

world* the_world = nullptr;

/** Sparse Array **/

SparseArray::Page SparseArray::Page::INVALID{0};

void SparseArray::initArrays(SparseArray* arrays, uint32_t count) {
    auto* pInvalidPage = &Page::INVALID;
    for (uint32_t j = 0; j < count; ++j) {
        auto* pages = arrays[j].pages;
        for (uint32_t i = 0; i < PAGES_MAX_COUNT; ++i) {
            pages[i] = pInvalidPage;
        }
    }
}

void SparseArray::clearArrays(SparseArray* arrays, uint32_t count) {
    auto* pInvalidPage = &Page::INVALID;
    for (uint32_t i = 0; i < count; ++i) {
        auto* pages = arrays[i].pages;
        for (uint32_t j = 0; j < PAGES_MAX_COUNT; ++j) {
            auto* page = pages[j];
            if (page != pInvalidPage) {
                free(page);
                pages[j] = pInvalidPage;
            }
        }
    }
}

void SparseArray::insert(Entity e, ComponentHandle handle) {
    ECXX_FULL_ASSERT(v != 0);
    auto* pages_ = pages;
    const auto page = e >> PageBits;
    const auto offset = e & PageMask;
    auto* indices = pages_[page]->indices;
    ECXX_FULL_ASSERT(pages_[page]->indices[offset] == 0);
    if (indices != Page::INVALID.indices) {
        indices[offset] = handle;
    } else {
        auto* page_data = new Page;
        memset(page_data, 0, PageSize);
        page_data->indices[offset] = handle;
        pages_[page] = page_data;
    }
}

// get `removed` element
// remove it at `removed` and assign to `target`
// returns handle
// `removed` and `target` should not be 0
ComponentHandle SparseArray::moveRemove(Entity removed, Entity target) {
    auto* pages_ = pages;

    // read: REMOVED entity data index (to relink DST entity to found index)
    const auto page = removed >> PageBits;
    auto* indices = pages_[page]->indices;
    const auto offset = removed & PageMask;
    const auto v = indices[offset];

    // write: link removed entity to zero data (we need to check entity HAS data)
    ECXX_FULL_ASSERT(v != 0);
    indices[offset] = 0;

    // write: relink DST entity to REMOVED entity data index
    pages_[target >> PageBits]->indices[target & PageMask] = v;

    return v;
}

void SparseArray::init() {
    initArrays(this, 1);
}

void SparseArray::clear() {
    clearArrays(this, 1);
}

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
    resetEntityPoolArrays(entities, generations);

    // reserve null entity
    entities[0] = 0;
    next = 1;
    size = 1;
}

void world::create(Entity* outEntities, uint32_t count_) {
    auto* indices = entities;
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
        auto* indices = entities;
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
            component->handleToEntity.reset();
            component->count = 1;
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
    component->count = 1;
    component->lockCounter = 0;
    component->data = userData;
    component->emplace = nullptr;
    component->erase = nullptr;
    component->clear = nullptr;
    component->entityToHandle.init();
    component->handleToEntity.reset();
}

}
