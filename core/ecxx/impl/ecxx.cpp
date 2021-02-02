#include "world.hpp"

#include <Tracy.hpp>

namespace ecs {

void entity_pool_reset(world* w) {
    w->count = 0;
    w->next = 0;
    auto* indices = w->indices;
    for (uint32_t i = 0; i < ENTITIES_MAX_COUNT; ++i) {
        indices[i] = i + 1;
    }
    memset(w->generations, 0, ENTITIES_MAX_COUNT);
}

void entity_alloc(world* w, Entity* entities, uint32_t count) {
    auto* indices = w->indices;
    auto next = w->next;

    for (uint32_t i = 0; i < count; ++i) {
        const auto nextIndex = indices[next];
        indices[next] = next;
        entities[i] = next;
        next = nextIndex;
    }
    w->next = next;
    w->count += count;
}

void entity_free(world* w, const Entity* entities, uint32_t count) {
    auto* indices = w->indices;
    auto* generations = w->generations;
    auto next = w->next;
    for (uint32_t i = 0; i < count; ++i) {
        const auto index = entities[i];
        indices[index] = next;
        ++generations[index];
        next = index;
    }
    w->next = next;
    w->count -= count;
}

/** Sparse Array **/

static SparseSetPage SS_INVALID_PAGE{0};

void sparse_sets_init(SparseSet* sets, uint32_t count) {
    auto* pInvalidPage = &SS_INVALID_PAGE;
    for (uint32_t j = 0; j < count; ++j) {
        auto* pages = sets[j].pages;
        for (uint32_t i = 0; i < SS_PAGES_MAX_COUNT; ++i) {
            pages[i] = pInvalidPage;
        }
    }
}

void sparse_sets_clear(SparseSet* sets, uint32_t count) {
    auto* pInvalidPage = &SS_INVALID_PAGE;
    for (uint32_t i = 0; i < count; ++i) {
        auto* pages = sets[i].pages;
        for (uint32_t j = 0; j < SS_PAGES_MAX_COUNT; ++j) {
            auto* page = pages[j];
            if (page != pInvalidPage) {
                free(page);
                pages[j] = pInvalidPage;
            }
        }
    }
}

void sparse_set_insert(SparseSet* set, Entity e, ComponentHandle handle) {
    ECXX_FULL_ASSERT(v != 0);
    auto* pages = set->pages;
    const auto page = e >> SS_PAGE_BITS;
    const auto offset = e & SS_PAGE_MASK;
    auto* indices = pages[page]->indices;
    ECXX_FULL_ASSERT(pages[page]->indices[offset] == 0);
    if (indices != SS_INVALID_PAGE.indices) {
        indices[offset] = handle;
    } else {
        auto* page_data = (SparseSetPage*) aligned_alloc(SS_PAGE_SIZE, SS_PAGE_SIZE);
        memset(page_data, 0, SS_PAGE_SIZE);
        page_data->indices[offset] = handle;
        pages[page] = page_data;
    }
}

//ComponentHandle sparse_set_get(const SparseSet* set, Entity e) {
//    return set->pages[e >> SS_PAGE_BITS]->indices[e & SS_PAGE_MASK];
//}

void sparse_set_set(SparseSet* set, Entity e, ComponentHandle handle) {
    set->pages[e >> SS_PAGE_BITS]->indices[e & SS_PAGE_MASK] = handle;
}

// get `removed` element
// remove it at `removed` and assign to `target`
// returns handle
// `removed` and `target` should not be 0
ComponentHandle sparse_set_move_remove(SparseSet* set, Entity removed, Entity target) {
    auto* pages = set->pages;

    // read: REMOVED entity data index (to relink DST entity to found index)
    const auto page = removed >> SS_PAGE_BITS;
    auto* indices = pages[page]->indices;
    const auto offset = removed & SS_PAGE_MASK;
    const auto v = indices[offset];

    // write: link removed entity to zero data (we need to check entity HAS data)
    ECXX_FULL_ASSERT(v != 0);
    indices[offset] = 0;

    // write: relink DST entity to REMOVED entity data index
    pages[target >> SS_PAGE_BITS]->indices[target & SS_PAGE_MASK] = v;

    return v;
}

// passport routines
bool passport_valid(const world* w, uint32_t passport) {
    const auto* generations = w->generations;
    const auto index = passport & INDEX_MASK;
    const auto generation = (passport >> INDEX_BITS) & GENERATION_MASK;
    return generation == generations[index];
}

// entity create / destroy
void entity_destroy(world* w, const Entity* entities, uint32_t count) {
    entity_free(w, entities, count);
    auto** components = w->components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components[i];
        // TODO: we can know all components ahead of time and register/init them and not check every case...
        if (component) {
            auto* erase = component->erase;
            const auto* entityToHandle = &component->entityToHandle;
            for (uint32_t j = 0; j < count; ++j) {
                const auto entity = entities[j];
                const auto handle = sparse_set_get(entityToHandle, entity);
                if (handle != 0) {
                    erase(component, entity);
                }
            }
        }
    }
}


// World create / destroy

void reserve_null_entity(world* w) {
    entity_alloc(w, &w->zero, 1);
    ECXX_FULL_ASSERT(w->zero == 0);
}

void world_initialize(world* w) {
    entity_pool_reset(w);
    memset(w->components, 0, COMPONENTS_MAX_COUNT * sizeof(void*));
    reserve_null_entity(w);
}

void world_reset(world* w) {
    entity_pool_reset(w);
    auto** components = w->components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components[i];
        if (component) {
            component->clear(component);
            sparse_sets_clear(&component->entityToHandle, 1);
            component->handleToEntity.resize(1);
            component->count = 1;
        }
    }
    reserve_null_entity(w);
}

void world_shutdown(world* w) {
    // skip clearing entity pool, because we don't need it anymore
    auto** components = w->components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components[i];
        if (component) {
            sparse_sets_clear(&component->entityToHandle, 1);
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
    sparse_sets_init(&component->entityToHandle, 1);
    component->handleToEntity.resize(1, 0);
}

}
