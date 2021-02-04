#pragma once

#include <cstdint>
#include <vector>

// TODO: disable at finish
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

/** Static configuration definition **/
inline constexpr uint32_t COMPONENTS_MAX_COUNT = 128;
inline constexpr uint32_t ENTITIES_MAX_COUNT = 0x10000;
inline constexpr uint32_t INDEX_BITS = 16;
inline constexpr uint32_t INDEX_MASK = 0xFFFF;
inline constexpr uint32_t GENERATION_MASK = 0xFF;

typedef uint16_t Entity;
typedef uint16_t ComponentHandle;
typedef uint16_t ComponentTypeId;
typedef uint8_t Generation;
typedef uint32_t Passport;

/** Sparse Array: map Entity to Component Handle **/

// min memory required:
// mem per vector = 32 (PagesCountMax) * 8 (ptr to page array) = 256 bytes
// for 128 components = 256 * 128 = 32768 bytes

// full page data size = 2048 (elements per page) * 2 (index size) = 4096 bytes
// all pages = 32 (pages for 65k entities) * 4096 = 131072 bytes

// FULL INDEX DATA memory for 128 component managers:
// (all pages + init mem) * 128 comps = (256 + 131072) * 128 = 16 809 984, 16 MB

// 0 is invalid index (null_value)

// 32 pages per component, so how many components at all we can track if pages will have global U16 id:
// - 65536 / 32 = 2048 components!

// size: 256 bytes
struct /* alignas(256) */ SparseArray {
    //inline constexpr uint32_t SS_PAGE_SIZE = 0x8000; // ~32kb index table per page
    static constexpr uint32_t PageSize = 0x1000; // general 4kb pages
    static constexpr uint32_t ELEMENTS_PER_PAGE = PageSize / sizeof(ComponentHandle); // 0x800 = 2048
    static constexpr uint32_t PageMask = ELEMENTS_PER_PAGE - 1; // 0x7FF
    static constexpr uint32_t PageBits = 11; // bit_count of page_mask

    // count on 32 pages for 16-bit entity index
    static constexpr uint32_t PAGES_MAX_COUNT = ENTITIES_MAX_COUNT / ELEMENTS_PER_PAGE;

    // `page_size` required to be power-of-two value
    static_assert(ELEMENTS_PER_PAGE > 0 && ((ELEMENTS_PER_PAGE & (ELEMENTS_PER_PAGE - 1)) == 0));

    struct /* alignas(4096) */ Page {
        static Page INVALID;

        ComponentHandle indices[ELEMENTS_PER_PAGE]; // 4096 bytes, page aligned
    };

    // 32 * 8 = 256
    Page* pages[PAGES_MAX_COUNT];

    [[nodiscard]]
    inline ComponentHandle get(Entity e) const {
        return pages[e >> PageBits]->indices[e & PageMask];
    }

    inline void set(Entity e, ComponentHandle handle) {
        pages[e >> PageBits]->indices[e & PageMask] = handle;
    }

    void insert(Entity e, ComponentHandle handle);

    [[nodiscard]]
    ComponentHandle moveRemove(Entity removed, Entity target);

    void init();

    void clear();

    static void initArrays(SparseArray* arrays, uint32_t count);

    static void clearArrays(SparseArray* arrays, uint32_t count);
};

struct EntityDenseArray {

    Entity* data;
    uint32_t size;
    uint32_t capacity;

    EntityDenseArray() {
        data = new Entity[64];
        *data = 0;
        size = 1;
        capacity = 64;
    }

    ~EntityDenseArray() {
        delete data;
    }

    void reset() {
        *data = 0;
        size = 1;
    }

    void grow() {
        const auto newCapacity = capacity << 1;
        auto* newData = new Entity[newCapacity];
        memcpy(newData, data, sizeof(Entity) * size);
        delete[] data;
        data = newData;
        capacity = newCapacity;
    }

    void push(Entity e) {
        if (size == capacity) {
            grow();
        }
        *(reinterpret_cast<Entity*>(data) + size) = e;
        ++size;
    }

    inline void set(ComponentHandle handle, Entity e) const {
        *(data + handle) = e;
    }

    [[nodiscard]]
    inline Entity get(ComponentHandle handle) const {
        return *(data + handle);
    }

    inline void remove_back() {
        --size;
    }

    [[nodiscard]]
    inline Entity back() const {
        return *(data + size - 1);
    }
};

/** Component Managers **/

struct ComponentRegistration {
    void* data; // 8 bytes

    ComponentHandle (* emplace)(void* component, Entity entity); // 8 bytes

    void (* erase)(void* component, Entity entity); // 8 bytes

    void (* clear)(void* component); // 8 bytes

    ComponentTypeId typeId; // 2 bytes
};

// 256 + 74 = 330 bytes
// 128 * 330 = 42'240 kb
struct ComponentHeader {
    SparseArray entityToHandle; // 256 bytes

    EntityDenseArray handleToEntity{}; // dynamic (8 + 4 + 4 ~ 16 bytes)

    uint32_t count; // 4 bytes
    uint32_t lockCounter; // 4 bytes

    void* data; // 8 bytes

    ComponentHandle (* emplace)(ComponentHeader* component, Entity entity); // 8 bytes

    void (* erase)(ComponentHeader* component, Entity entity); // 8 bytes

    void (* clear)(ComponentHeader* component); // 8 bytes

    ComponentTypeId typeId; // 2 bytes
};

void component_type_init(ComponentHeader* component, ComponentTypeId typeId, void* userData);

/** Identity Index generator **/
struct ComponentTypeIdCounter {
    static ComponentTypeId counter;

    inline static ComponentTypeId next() {
        return counter++;
    }
};

inline ComponentTypeId ComponentTypeIdCounter::counter = 0;

// 1. match between TU
// 2. starts from 0 for each Identity type
template<typename T>
struct ComponentTypeIdGenerator {
    static const ComponentTypeId value;
};

template<typename T>
inline const ComponentTypeId ComponentTypeIdGenerator<T>::value = ComponentTypeIdCounter::next();

template<typename Component>
constexpr static inline ComponentTypeId type() noexcept {
    return ComponentTypeIdGenerator<Component>::value;
}

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
    //alignas(4096)
    Entity entities[ENTITIES_MAX_COUNT]; // 2 * ENTITIES_MAX_COUNT = 131072 bytes

    // entity generations
    //alignas(4096)
    Generation generations[ENTITIES_MAX_COUNT]; // 65536 bytes

    // per component, data manager
    //alignas(1024)
    ComponentHeader* components[COMPONENTS_MAX_COUNT]; // 8 * 128 = 1024 bytes

    // per component, map entity to data handle
//    alignas(1024) SparseArray* pEntityToHandle[COMPONENTS_MAX_COUNT]; // 8 * 128 = 1024 bytes

    //// control section
    // allocated entities count
    uint32_t size;

    // next free index in entity pool
    Entity next;

    // just zero entity (reserved null entity)
    Entity zero;

    [[nodiscard]]
    inline Generation generation(Entity e) const {
        return generations[e];
    }

    [[nodiscard]]
    inline bool isAllocated(Entity e) const {
        return entities[e] == e;
    }


    /** lifecycle **/

    void initialize();

    void reset();

    void shutdown();

    /** entity pool **/

// entity create / destroy
    void create(Entity* outEntities, uint32_t count);

    void destroy(const Entity* entitiesToDestroy, uint32_t count);

    template<typename ...Component>
    void create(Entity* outEntities, uint32_t count);

    // passport
    [[nodiscard]]
    bool check(Passport passport) const;

    [[nodiscard]]
    inline bool isValid(Entity e) const {
        return e && entities[e] == e;
    }

    /** components **/

    template<typename Component>
    [[nodiscard]] inline bool has(Entity e) const;

    template<typename Component>
    [[nodiscard]] inline Component& get(Entity e) const;

    template<typename Component>
    [[nodiscard]] inline Component* tryGet(Entity e) const;

private:
    void resetEntityPool();
};

inline ComponentHeader* world_component_type(const world* w, ComponentTypeId componentTypeId) {
    ECXX_FULL_ASSERT(componentTypeId < COMPONENTS_MAX_COUNT);
    auto* component = w->components[componentTypeId];
    ECXX_FULL_ASSERT(component != nullptr);
    return component;
}

inline void world_register_component(world* w, ComponentHeader* component) {
    w->components[component->typeId] = component;
}

// world create / destroy

inline ComponentHandle entity_get(const world* w, Entity e, ComponentTypeId componentId) {
    const auto* component = world_component_type(w, componentId);
    return component->entityToHandle.get(e);
}

inline ComponentHandle entity_get_or_create(world* w, Entity e, ComponentTypeId componentId) {
    auto* component = world_component_type(w, componentId);
    const auto handle = component->entityToHandle.get(e);
    if (handle == 0) {
        return component->emplace(component, e);
    }
    return handle;
}

extern world* the_world;

/** Templated generic **/

template<typename DataType>
class ComponentStorage final {
public:
    ComponentHeader component{};
    std::vector<DataType> data{1};

    static constexpr bool has_data = std::negation_v<typename std::is_empty<DataType>::type>;

    ComponentStorage() {
        component_type_init(&component, type<DataType>(), this);
        component.emplace = ComponentStorage<DataType>::s_emplace;
        component.erase = ComponentStorage<DataType>::s_erase;
        component.clear = ComponentStorage<DataType>::s_clear;
    }

    template<typename ...Args>
    DataType& emplace(Entity entity, Args&& ...args) {
        auto& entityToHandle = component.entityToHandle;
        auto& handleToEntity = component.handleToEntity;
        const auto handle = component.count;
        ECXX_ASSERT(component.lockCounter == 0);
        ECXX_ASSERT(entityToHandle.get(entity) == 0);

        entityToHandle.insert(entity, handle);
        handleToEntity.push(entity);
        ++component.count;
        if constexpr (has_data) {
            if constexpr (std::is_aggregate_v<DataType>) {
                return data.emplace_back(DataType{args...});
            } else {
                return data.emplace_back(args...);
            }
        } else {
            return data[0u];
        }
    }

    ComponentHandle emplace_default(Entity entity) {
        auto& entityToHandle = component.entityToHandle;
        auto& handleToEntity = component.handleToEntity;
        const auto handle = component.count;
        ECXX_ASSERT(component.lockCounter == 0);
        ECXX_ASSERT(entityToHandle.get(entity) == 0);

        entityToHandle.insert(entity, handle);
        handleToEntity.push(entity);
        ++component.count;
        if constexpr (has_data) {
            data.emplace_back();
            return handle;
        } else {
            return 0;
        }
    }

    void erase(Entity entity) {
        ECXX_ASSERT(component.lockCounter == 0);
        ECXX_ASSERT(component.entityToHandle.get(entity) != 0);
        const auto backEntity = component.handleToEntity.back();
        if (entity != backEntity) {
            erase_from_middle(entity, backEntity);
        } else {
            erase_from_back(entity);
        }
    }

    void erase_from_middle(Entity e, Entity last) {
        --component.count;
        const auto handle = component.entityToHandle.moveRemove(e, last);
        component.handleToEntity.set(handle, last);
        component.handleToEntity.remove_back();
        if constexpr (has_data) {
            data[handle] = std::move(data.back());
            data.pop_back();
        }
    }

    void erase_from_back(Entity e) {
        --component.count;
        component.entityToHandle.set(e, 0);
        component.handleToEntity.remove_back();
        if constexpr (has_data) {
            data.pop_back();
        }
    }

    inline DataType& get(Entity e) const {
        if constexpr (has_data) {
            return const_cast<DataType&>(data[component.entityToHandle.get(e)]);
        } else {
            return const_cast<DataType&>(data[0]);
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
            return data.data() + handle;
        } else {
            return data.data();
        }
    }

    DataType& get_or_default_by_handle(ComponentHandle handle) const {
        if constexpr (has_data) {
            return get_data(handle);
        } else {
            return get_data(0u);
        }
    }

    /** Methods used for dynamic operations instead of using virtual calls **/
    static ComponentHandle s_emplace(ComponentHeader* hdr, Entity e) {
        return static_cast<ComponentStorage*>(hdr->data)->emplace_default(e);
    }

    static void s_erase(ComponentHeader* hdr, Entity entity) {
        static_cast<ComponentStorage*>(hdr->data)->erase(entity);
    }

    static void s_clear(ComponentHeader* hdr) {
        static_cast<ComponentStorage*>(hdr->data)->data.resize(1);
    }

    inline DataType& get_data(ComponentHandle i) const {
        return const_cast<DataType&>(data[i]);
    }

    // get data by packed index
    inline DataType& get_data_by_index(ComponentHandle i) const {
        if constexpr (has_data) {
            return const_cast<DataType&>(data[i]);
        } else {
            return const_cast<DataType&>(data[0]);
        }
    }
};

template<typename Component>
inline ComponentStorage<Component>* tpl_world_storage(const world* w) {
    const auto componentId = type<Component>();
    const auto* component = w->components[componentId];
    return static_cast<ComponentStorage<Component>*>(component->data);
}

template<typename Component, typename ...Args>
inline Component& entity_assign(world* w, Entity index, Args&& ... args) {
    auto* storage = tpl_world_storage<Component>(w);
    ECXX_FULL_ASSERT(storage != nullptr);
    return storage->emplace(index, args...);
}

template<typename Component, typename ...Args>
inline void entity_assign(world* w, Entity* entities, uint32_t count, Args&& ... args) {
    auto* storage = tpl_world_storage<Component>(w);
    ECXX_FULL_ASSERT(storage != nullptr);
    for (uint32_t i = 0; i < count; ++i) {
        storage->emplace(entities[i], args...);
    }
}

template<typename Component, typename ...Args>
inline Component& entity_reassign(world* w, Entity entity, Args&& ... args) {
    auto* component = world_component_type(w, type<Component>());
    const auto handle = component->entityToHandle.get(entity);
    auto* storage = static_cast<ComponentStorage<Component>*>(component->data);
    if (handle != 0) {
        auto& data = storage->get_by_handle(handle);
        data = {args...};
        return data;
    }
    return storage->emplace(entity, args...);
}

template<typename Component>
inline bool world::has(Entity e) const {
    const auto* component = world_component_type(this, type<Component>());
    const auto& entityToHandle = component->entityToHandle;
    const auto handle = entityToHandle.get(e);
    return handle != 0;
}

template<typename Component>
inline Component& world::get(Entity e) const {
    return tpl_world_storage<Component>(this)->get(e);
}

template<typename Component>
inline Component* world::tryGet(Entity e) const {
    auto* storage = tpl_world_storage<Component>(this);
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        return storage->get_ptr_by_handle(handle);
    }
    return nullptr;
}

template<typename Component>
inline void entity_remove(world* w, Entity e) {
    tpl_world_storage<Component>(w)->erase(e);
}

template<typename Component>
inline bool entity_try_remove(world* w, Entity e) {
    auto* storage = tpl_world_storage<Component>(w);
    if (storage->component.entityToHandle.get(e) != 0) {
        storage->erase(e);
        return true;
    }
    return false;
}

template<typename Component>
inline Component& entity_get_or_create(world* w, Entity e) {
    auto* storage = tpl_world_storage<Component>(w);
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        return storage->get_by_handle(handle);
    }
    return storage->emplace(e);
}


template<typename Component>
inline const Component& entity_get_or_default(world* w, Entity e) {
    const auto* storage = tpl_world_storage<Component>(w);
    const auto handle = storage->component.entityToHandle.get(e);
    return storage->get_or_default_by_handle(handle);
}

template<typename ...Component>
inline void world::create(Entity* outEntities, uint32_t count) {
    create(outEntities, count);
    (entity_assign<Component>(this, outEntities, count), ...);
}

template<typename Component>
inline void tpl_world_register(world* w) {
    auto* storage = new ComponentStorage<Component>();
    world_register_component(w, &storage->component);
}

}