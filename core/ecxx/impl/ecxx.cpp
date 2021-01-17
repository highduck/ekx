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

static SparseArrayPage SA_INVALID_PAGE{0};

void sparse_arrays_init(SparseArray* arrays) {
    auto* invalid_page_ptr = &SA_INVALID_PAGE;
    for (uint32_t j = 0; j < COMPONENTS_MAX_COUNT; ++j) {
        auto* pages = arrays[j].pages;
        for (uint32_t i = 0; i < SA_PAGES_MAX_COUNT; ++i) {
            pages[i] = invalid_page_ptr;
        }
        memset(arrays[j].masks, 0, SA_PAGES_MAX_COUNT * sizeof(uint16_t));
    }
}

void sparse_arrays_clear(SparseArray* arrays) {
    auto* invalid_page_ptr = &SA_INVALID_PAGE;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* masks = arrays[i].masks;
        auto* pages = arrays[i].pages;
        for (uint32_t j = 0; j < SA_PAGES_MAX_COUNT; ++j) {
            if (masks[j] != 0) {
                masks[j] = 0;
                TracyFreeN(pages[j], "ecs::sparse_page");
                free(pages[j]);
                pages[j] = invalid_page_ptr;
            }
        }
    }
}

void sparse_array_insert(SparseArray* array, Entity i, ComponentHandle v) {
    ECXX_FULL_ASSERT(v != 0);
    auto* masks = array->masks;
    auto* pages = array->pages;
    const auto page = i >> SA_PAGE_BITS;
    const auto offset = i & SA_PAGE_MASK;
    const auto mask = masks[page];
    ECXX_FULL_ASSERT(pages[page]->indices[offset & mask] == 0);
    if (mask != 0) {
        pages[page]->indices[offset] = v;
    } else {
        auto* page_data = (SparseArrayPage*) aligned_alloc(SA_PAGE_SIZE, SA_PAGE_SIZE);
        memset(page_data, 0, SA_PAGE_SIZE);
        page_data->indices[offset] = v;
        pages[page] = page_data;
        masks[page] = SA_PAGE_MASK;
    }
}

ComponentHandle sparse_array_get(const SparseArray* array, Entity i) {
    const auto page_index = i >> SA_PAGE_BITS;
    return array->pages[page_index]->indices[i & array->masks[page_index]];
}

void sparse_array_remove(SparseArray* array, Entity i) {
    const auto page = i >> SA_PAGE_BITS;
    const auto offset = i & array->masks[page];
    ECXX_FULL_ASSERT(array->pages[page]->indices[offset] != 0);
    array->pages[page]->indices[offset] = 0;
}

// get `i` element
// if i == dst - remove it from `i`
// else - remove it at `i` and assign to `dst`
// returns element
// i and dst should not be 0
ComponentHandle sparse_array_move_remove(SparseArray* array, Entity i, Entity dst) {
    auto* pages = array->pages;
    auto* masks = array->masks;
    const auto page = i >> SA_PAGE_BITS;
    const auto offset = i & masks[page];
    auto* indices = pages[page];
    const auto v = indices->indices[offset];
    ECXX_FULL_ASSERT(v != 0);
    indices->indices[offset] = 0;
    if (i != dst) {
        const auto dst_page_index = dst >> SA_PAGE_BITS;
        pages[dst_page_index]->indices[dst & masks[dst_page_index]] = v;
    }
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
    auto* maps = w->maps;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components[i];
        // TODO: we can know all components ahead of time and register/init them and not check every case...
        if (component) {
            for (uint32_t j = 0; j < count; ++j) {
                const auto index = entities[j];
                const auto component_index = sparse_array_get(maps + i, index);
                if (component_index != 0) {
                    component->erase_dyn(index);
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
    sparse_arrays_init(w->maps);
    memset(w->components, 0, COMPONENTS_MAX_COUNT * sizeof(void*));
    reserve_null_entity(w);
}

void world_reset(world* w) {
    entity_pool_reset(w);
    sparse_arrays_clear(w->maps);

    auto** components = w->components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        auto* component = components[i];
        if (component) {
            component->clear();
        }
    }
    reserve_null_entity(w);
}

void world_shutdown(world* w) {
    // skip clearing entity pool, because we don't need it anymore
    sparse_arrays_clear(w->maps);
    auto** components = w->components;
    for (uint32_t i = 0; i < COMPONENTS_MAX_COUNT; ++i) {
        TracyFreeN(components[i], "ecs::ComponentManager");
        delete components[i];
    }
}

}
