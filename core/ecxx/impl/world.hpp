#pragma once

#include <cstdint>
#include <vector>

// enable to force asserts
//#define ECXX_ENABLE_ASSERT
// enable to force pedantic asserts
//#define ECXX_ENABLE_ASSERT_PEDANTIC

#ifndef NDEBUG
#define ECXX_ENABLE_ASSERT
#endif

#if defined(ECXX_ENABLE_ASSERT_PEDANTIC) || defined(ECXX_ENABLE_ASSERT)

#include <stdio.h>

#define ecxx_on_assert(e, file, line) ((void)printf("%s:%d: failed assertion `%s'\n", file, line, e), abort())
#define ecxx_assert(e) (!(e) ? ecxx_on_assert(#e, __FILE__, __LINE__) : ((void)0))
#else

#define ecxx_assert(e) ((void)0)

#endif

#ifdef ECXX_ENABLE_ASSERT
#define ECXX_ASSERT(e) ecxx_assert(e)
#else
#define ECXX_ASSERT(ignore) ((void)0)
#endif

#ifdef ECXX_ENABLE_ASSERT_PEDANTIC
#define ECXX_FULL_ASSERT(e) ecxx_assert(e)
#else
#define ECXX_FULL_ASSERT(ignore) ((void)0)
#endif

namespace ecs {

/** Identity Index generator **/
struct identity_counter {
    inline static uint32_t counter{0};
};

// 1. match between TU
// 2. starts from 0 for each Identity type
template<typename T>
struct identity_generator {
    static const uint32_t value;
};

template<typename T>
inline const uint32_t identity_generator<T>::value = identity_counter::counter++;

template<typename Component>
constexpr static inline unsigned type() noexcept {
    return identity_generator<Component>::value;
}

/** Static configuration definition **/
inline constexpr uint32_t COMPONENTS_MAX_COUNT = 128;
inline constexpr uint32_t ENTITIES_MAX_COUNT = 0x10000;
inline constexpr uint32_t INDEX_BITS = 16;
inline constexpr uint32_t INDEX_MASK = 0xFFFF;
inline constexpr uint32_t GENERATION_MASK = 0xFF;

typedef uint16_t Entity;
typedef uint16_t ComponentHandle;
typedef uint8_t Generation;
typedef uint32_t Passport;

/** Sparse Array: map Entity to Component Handle **/

//inline constexpr uint32_t SA_PAGE_SIZE = 0x8000; // ~32kb index table per page
inline constexpr uint32_t SA_PAGE_SIZE = 0x1000; // general 4kb pages
inline constexpr uint32_t SA_ELEMENTS_PER_PAGE = SA_PAGE_SIZE / sizeof(ComponentHandle); // 0x800 = 2048
static constexpr uint32_t SA_PAGE_MASK = SA_ELEMENTS_PER_PAGE - 1; // 0x7FF
static constexpr uint32_t SA_PAGE_BITS = 11; // bit_count of page_mask

static constexpr uint32_t SA_PAGES_MAX_COUNT = ENTITIES_MAX_COUNT / SA_ELEMENTS_PER_PAGE; // 32

// `page_size` required to be power-of-two value
static_assert(SA_ELEMENTS_PER_PAGE > 0 && ((SA_ELEMENTS_PER_PAGE & (SA_ELEMENTS_PER_PAGE - 1)) == 0));

// min memory required:
// mem per vector = 32 (PagesCountMax) * (8 (ptr to page array) + 2 (page_mask_size)) = 320 bytes
// for 128 components = 320 * 128 = 40960 bytes

// full page data size = 16384 (elements per page) * 2 (index size) = ~32kb
// all pages = 4 (pages for 65k entities) * 32 kb = ~131 kb

// FULL INDEX DATA memory for 128 component managers:
// (all pages + init mem) * 128 comps = 16 782 336, 16 MB

// 0 is invalid index (null_value)

struct SparseArrayPage {
    alignas(4096) ComponentHandle indices[SA_ELEMENTS_PER_PAGE]; // 4096 bytes, page aligned
};

// size: 320 bytes
struct SparseArray {
    alignas(128) uint16_t masks[SA_PAGES_MAX_COUNT];
    alignas(128) SparseArrayPage* pages[SA_PAGES_MAX_COUNT];
};

void sparse_array_insert(SparseArray* array, Entity i, ComponentHandle v);

ComponentHandle sparse_array_get(const SparseArray* array, Entity i);

void sparse_array_remove(SparseArray* array, Entity i);

ComponentHandle sparse_array_move_remove(SparseArray* array, Entity i, Entity dst);

/** Component Managers **/

// 320 + 32 = 352 bytes
struct DataManagerState {
    SparseArray entityToHandle; // 320 bytes
    std::vector<Entity> handleToEntity{}; // dynamic (8 + 8 + 8 ~ 24-32 bytes)
    uint32_t size; // 4 bytes
    uint32_t lock_counter; // 4 bytes
};

class entity_map_base {
public:
    explicit entity_map_base(SparseArray* entityToHandle_) : entityToHandle{entityToHandle_} {
        handleToEntity.push_back(0);
    }

    virtual ~entity_map_base() = default;

    virtual void emplace_dyn(Entity) = 0;

    virtual void erase_dyn(Entity) = 0;

    virtual void clear() = 0;

    SparseArray* entityToHandle;
    std::vector<Entity> handleToEntity;
    uint32_t count = 1;
    uint32_t lock_counter = 0;
};

template<typename DataType>
class entity_map final : public entity_map_base {
public:
    static constexpr bool has_data = std::negation_v<typename std::is_empty<DataType>::type>;

    explicit entity_map(SparseArray* entityToHandle_) : entity_map_base(entityToHandle_) {
        data_.emplace_back();
    }

    ~entity_map() override = default;

    template<typename ...Args>
    DataType& emplace(Entity idx, Args&& ...args) {
        //bool locked_debug = locked();
        ECXX_ASSERT(lock_counter == 0);
        ECXX_ASSERT(sparse_array_get(entityToHandle, idx) == 0);

        sparse_array_insert(entityToHandle, idx, count++);
        handleToEntity.push_back(idx);
        if constexpr (has_data) {
            if constexpr (std::is_aggregate_v<DataType>) {
                return data_.emplace_back(DataType{args...});
            } else {
                return data_.emplace_back(args...);
            }
        } else {
            return data_[0u];
        }
    }

    void erase(Entity idx) {
        ECXX_ASSERT(lock_counter == 0);
        ECXX_ASSERT(sparse_array_get(entityToHandle, idx) != 0);

        --count;
        const auto back_entity = handleToEntity.back();
        const auto index = sparse_array_move_remove(entityToHandle, idx, back_entity);
        handleToEntity[index] = back_entity;
        handleToEntity.pop_back();
        if constexpr (has_data) {
            data_[index] = std::move(data_.back());
            data_.pop_back();
        }
    }

    inline DataType& get(Entity idx) const {
        if constexpr (has_data) {
            return get_data(sparse_array_get(entityToHandle, idx));
        } else {
            return get_data(0u);
        }
    }

    inline DataType& get_by_handle(ComponentHandle handle) const {
        if constexpr (has_data) {
            return get_data(handle);
        } else {
            return get_data(0u);
        }
    }

    inline DataType* get_ptr_by_handle(ComponentHandle handle) {
        if constexpr (has_data) {
            return data_.data() + handle;
        } else {
            return data_.data();
        }
    }

    DataType& get_or_default_by_handle(ComponentHandle handle) const {
        if constexpr (has_data) {
            return get_data(handle);
        } else {
            return get_data(0u);
        }
    }

    void emplace_dyn(Entity idx) final {
        emplace(idx);
    }

    void erase_dyn(Entity idx) final {
        erase(idx);
    }

    void clear() final {
        handleToEntity.resize(1);
        data_.resize(1);
        count = 1;
    }

    inline DataType& get_data(ComponentHandle i) const {
        return const_cast<DataType&>(data_[i]);
    }

    // get data by packed index
    inline DataType& get_data_by_index(ComponentHandle i) const {
        if constexpr (has_data) {
            return const_cast<DataType&>(data_[i]);
        } else {
            return const_cast<DataType&>(data_[0]);
        }
    }

    std::vector<DataType> data_;
};

/**
* Entity Pool generates dense indices for Entity representation.
* We track Generations on reusing deleted entities.
* Generation is 8-bit, so I swear to check references next update.
* Collision is possible if in one frame we do destroy and create more than 256 times
*/

/** World **/
struct world {

    // entity pool data, indices and generations
    // SIZE: 200 kb

    // entity indices
    alignas(4096) Entity indices[ENTITIES_MAX_COUNT]; // 2 * ENTITIES_MAX_COUNT = 131072 bytes

    // entity generations
    alignas(4096) Generation generations[ENTITIES_MAX_COUNT]; // 65536 bytes

    // per component, map entity to data handle
    alignas(4096) SparseArray maps[COMPONENTS_MAX_COUNT]; // 128 * 352 bytes = 45056 bytes = 45 kb

    // per component, data manager
    alignas(4096) entity_map_base* components[COMPONENTS_MAX_COUNT]; // 128 * 8 bytes = 1024 bytes = 1 kb

    //// control section
    // allocated entities count
    uint32_t count;

    // next free index in entity pool
    Entity next;

    // just zero entity (reserved null entity)
    Entity zero;
};

// entity create / destroy

void entity_alloc(world* w, Entity* entities, uint32_t count);

template<typename ...Component>
void entity_create(world* w, Entity* entities, uint32_t count);

void entity_destroy(world* w, const Entity* entities, uint32_t count);

// passport
bool passport_valid(const world* w, uint32_t passport);

inline Generation entity_generation(world* w, Entity index) {
    return w->generations[index];
}

// world create / destroy

void world_initialize(world* w);

void world_reset(world* w);

void world_shutdown(world* w);

template<typename Component>
inline SparseArray* world_get_index_map(const world* w) {
    const auto component_type_id = type<Component>();
    ECXX_FULL_ASSERT(component_type_id < COMPONENTS_MAX_COUNT);
    return w->maps + component_type_id;
}

template<typename Component>
inline entity_map<Component>* world_tryGetComponents(const world* w) {
    const auto component_type_id = type<Component>();
    ECXX_ASSERT(component_type_id < COMPONENTS_MAX_COUNT);
    return static_cast<entity_map<Component>*>(w->components[component_type_id]);
}

template<typename Component>
[[nodiscard]] inline bool component_is_locked(const world* w) {
    const auto* pool = world_tryGetComponents<Component>(w);
    return pool && pool->lock_counter != 0;
}

template<typename Component>
static entity_map<Component>& component_ensure(world* w) {
    const auto cid = type<Component>();
    ECXX_ASSERT(cid < COMPONENTS_MAX_COUNT);

    auto* map = w->components[cid];
    if (map) {
    } else {
        map = new entity_map<Component>(w->maps + cid);
        w->components[cid] = map;
    }
    return *static_cast<entity_map<Component>*>(map);
}

template<typename Component, typename ...Args>
inline Component& entity_assign(world* w, Entity index, Args&& ... args) {
    return component_ensure<Component>(w).emplace(index, args...);
}

template<typename Component, typename ...Args>
inline void entity_assign(world* w, Entity* entities, uint32_t count, Args&& ... args) {
    auto& component_manager = component_ensure<Component>(w);
    for (uint32_t i = 0; i < count; ++i) {
        component_manager.emplace(entities[i], args...);
    }
}

template<typename Component, typename ...Args>
inline Component& entity_reassign(world* w, Entity index, Args&& ... args) {
    const auto component_type_id = type<Component>();
    const auto component_handle = sparse_array_get(w->maps + component_type_id, index);
    auto& pool = component_ensure<Component>(w);
    if (component_handle != 0) {
        auto& data = pool.get_by_handle(component_handle);
        data = {args...};
        return data;
    }
    return pool.emplace(index, args...);
}

template<typename Component>
[[nodiscard]] inline bool entity_has(const world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    return sparse_array_get(w->maps + component_type_id, idx) != 0;
}

template<typename Component>
inline const Component& entity_get(const world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    const auto component_handle = sparse_array_get(w->maps + component_type_id, idx);
    ECXX_ASSERT(component_handle != 0);
    return static_cast<entity_map<Component>*>(w->components[component_type_id])->get_by_handle(component_handle);
}

template<typename Component>
inline Component* entity_tryGet(world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    const auto component_handle = sparse_array_get(w->maps + component_type_id, idx);
    if (component_handle != 0) {
        return static_cast<entity_map<Component>*>(w->components[component_type_id])->
                get_ptr_by_handle(component_handle);
    }
    return nullptr;
}

template<typename Component>
inline Component& entity_get(world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    const auto component_handle = sparse_array_get(w->maps + component_type_id, idx);
    ECXX_ASSERT(component_handle != 0);
    return static_cast<entity_map<Component>*>(w->components[component_type_id])->get_by_handle(component_handle);
}

template<typename Component>
inline Component& entity_get_or_create(world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    auto& component_manager = component_ensure<Component>(w);
    const auto component_handle = sparse_array_get(w->maps + component_type_id, idx);
    if (component_handle == 0) {
        return component_manager.emplace(idx);
    } else {
        return component_manager.get_by_handle(component_handle);
    }
}

template<typename Component>
inline const Component& entity_get_or_default(world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    auto& component_manager = component_ensure<Component>(w);
    const auto component_handle = sparse_array_get(w->maps + component_type_id, idx);
    return component_manager.get_or_default_by_handle(component_handle);
}

template<typename Component>
inline void entity_remove(world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    ECXX_ASSERT(sparse_array_get(w->maps + component_type_id, idx) != 0);
    static_cast<entity_map<Component>*>(w->components[component_type_id])->erase(idx);
}

template<typename Component>
inline bool entity_try_remove(world* w, Entity idx) {
    const auto component_type_id = type<Component>();
    if (sparse_array_get(w->maps + component_type_id, idx) != 0) {
        static_cast<entity_map<Component>*>(w->components[component_type_id])->erase(idx);
        return true;
    }
    return false;
}

//inline bool isAlive(world* w, Entity e) {
//    return w->indices[e] == e;
//}

inline bool entity_is_allocated(world* w, Entity e) {
    return w->indices[e] == e;
}

inline world the_world{};

template<typename ...Component>
inline void entity_create(world* w, Entity* entities, uint32_t count) {
    entity_alloc(w, entities, count);
    (entity_assign<Component>(w, entities, count), ...);
}

}