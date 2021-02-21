#pragma once

#include <stdint.h>
#include <ek/ds/SparseArray.hpp>
#include <ek/ds/DynArray.hpp>
#include <ek/assert.hpp>
#include <vector>

namespace ecs {

/** Static configuration definition **/
inline constexpr uint32_t COMPONENTS_MAX_COUNT = 128;
inline constexpr uint32_t ENTITIES_MAX_COUNT = 0x10000;
inline constexpr uint32_t INDEX_BITS = 16;
inline constexpr uint32_t INDEX_MASK = 0xFFFF;
inline constexpr uint32_t GENERATION_MASK = 0xFF;

typedef uint16_t Entity;
typedef uint8_t Generation;
//typedef uint8_t WorldIndex;
typedef uint32_t Passport; // Passport is compressed unique ID with information for: world index, entity index, entity generation

typedef uint16_t ComponentHandle;
typedef uint16_t ComponentTypeId;

/** Component Managers **/

struct ComponentRegistration {
    void* data; // 8 bytes

    ComponentHandle (* emplace)(void* component, Entity entity); // 8 bytes

    void (* erase)(void* component, Entity entity); // 8 bytes

    void (* clear)(void* component); // 8 bytes

    ComponentTypeId typeId; // 2 bytes
};

using EntityLookup = ek::SparseArray<Entity, ComponentHandle, ENTITIES_MAX_COUNT>;

// 256 + 74 = 330 bytes
// 128 * 330 = 42'240 kb
struct ComponentHeader {
    EntityLookup entityToHandle; // 256 bytes

    ek::DynArray<Entity> handleToEntity{}; // dynamic (8 + 4 + 4 ~ 16 bytes)

    void* data; // 8 bytes

    ComponentHandle (* emplace)(ComponentHeader* component, Entity entity); // 8 bytes

    void (* erase)(ComponentHeader* component, Entity entity); // 8 bytes

    void (* clear)(ComponentHeader* component); // 8 bytes

    uint32_t lockCounter; // 4 bytes

    ComponentTypeId typeId; // 2 bytes

    [[nodiscard]] inline unsigned count() const {
        return handleToEntity.size;
    }
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

template<typename DataType>
class ComponentStorage;

/** World **/
struct world {

    // entity pool data, indices and generations
    // SIZE: 200 kb

    // entity indices
    //alignas(4096)
    Entity entityPool[ENTITIES_MAX_COUNT]; // 2 * ENTITIES_MAX_COUNT = 131072 bytes

    // entity generations
    //alignas(4096)
    Generation generations[ENTITIES_MAX_COUNT]; // 65536 bytes

    //EntityLookup componentEntityToHandle[COMPONENTS_MAX_COUNT]; // 256 * 128 = 32768 bytes

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
        return entityPool[e] == e;
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
        return e && entityPool[e] == e;
    }

    /** components **/

    template<typename Component>
    [[nodiscard]] inline bool has(Entity e) const;

    template<typename Component>
    [[nodiscard]] inline Component& get(Entity e) const;

    template<typename Component>
    [[nodiscard]] inline Component* tryGet(Entity e) const;

    template<typename Component, typename ...Args>
    inline Component& assign(Entity e, Args&& ... args) const;

    template<typename Component, typename ...Args>
    inline void assignBatch(Entity* entities, uint32_t count, Args&& ... args) const;

    template<typename Component, typename ...Args>
    inline Component& reassign(Entity e, Args&& ... args) const;

    [[nodiscard]]
    inline ComponentHeader* getComponentHeader(ComponentTypeId componentTypeId) const {
        EK_ASSERT_R2(componentTypeId < COMPONENTS_MAX_COUNT);
        auto* component = *(components + componentTypeId);
        EK_ASSERT_R2(component != nullptr);
        return component;
    }

    template<typename Component>
    inline ComponentStorage<Component>* getStorage() const;

    [[nodiscard]]
    inline ComponentHandle get(Entity e, ComponentTypeId componentId) const {
        return getComponentHeader(componentId)->entityToHandle.get(e);
    }

    [[nodiscard]]
    inline ComponentHandle getOrCreate(Entity e, ComponentTypeId componentId) const {
        auto* component = getComponentHeader(componentId);
        const auto handle = component->entityToHandle.get(e);
        if (handle == 0) {
            return component->emplace(component, e);
        }
        return handle;
    }

    template<typename Component>
    inline void remove(Entity e) const;

    template<typename Component>
    inline bool tryRemove(Entity e) const;

    template<typename Component>
    inline Component& getOrCreate(Entity e) const;

    template<typename Component>
    inline const Component& getOrDefault(Entity e) const;

    inline void registerComponent(ComponentHeader* component) {
        components[component->typeId] = component;
    }

    template<typename Component>
    inline void registerComponent();

private:
    void resetEntityPool();
};

extern world the_world;

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
        const auto handle = component.count();
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(entityToHandle.get(entity) == 0);

        entityToHandle.insert(entity, handle);
        handleToEntity.push_back(entity);
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
        const auto handle = component.count();
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(entityToHandle.get(entity) == 0);

        entityToHandle.insert(entity, handle);
        handleToEntity.push_back(entity);
        if constexpr (has_data) {
            data.emplace_back();
            return handle;
        } else {
            return 0;
        }
    }

    void erase(Entity entity) {
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(component.entityToHandle.get(entity) != 0);
        const auto backEntity = component.handleToEntity.back();
        if (entity != backEntity) {
            erase_from_middle(entity, backEntity);
        } else {
            erase_from_back(entity);
        }
    }

    void erase_from_middle(Entity e, Entity last) {
        const auto handle = component.entityToHandle.moveRemove(e, last);
        component.handleToEntity.set(handle, last);
        component.handleToEntity.remove_back();
        if constexpr (has_data) {
            data[handle] = std::move(data.back());
            data.pop_back();
        }
    }

    void erase_from_back(Entity e) {
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
inline ComponentStorage<Component>* world::getStorage() const {
    auto* component = components[type<Component>()];
    EK_ASSERT_R2(component != nullptr);
    return static_cast<ComponentStorage<Component>*>(component->data);
}

template<typename Component, typename ...Args>
inline Component& world::assign(Entity e, Args&& ... args) const {
    return getStorage<Component>()->emplace(e, args...);
}

template<typename Component, typename ...Args>
inline void world::assignBatch(Entity* entities, uint32_t count, Args&& ... args) const {
    auto* storage = getStorage<Component>();
    for (uint32_t i = 0; i < count; ++i) {
        storage->emplace(entities[i], args...);
    }
}

template<typename Component, typename ...Args>
inline Component& world::reassign(Entity e, Args&& ... args) const {
    ComponentStorage<Component>* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        auto& data = storage->get_by_handle(handle);
        data = {args...};
        return data;
    }
    return storage->emplace(e, args...);
}

template<typename Component>
inline bool world::has(Entity e) const {
    const auto* component = getComponentHeader(type<Component>());
    const auto& entityToHandle = component->entityToHandle;
    const auto handle = entityToHandle.get(e);
    return handle != 0;
}

template<typename Component>
inline Component& world::get(Entity e) const {
    return getStorage<Component>()->get(e);
}

template<typename Component>
inline Component* world::tryGet(Entity e) const {
    auto* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        return storage->get_ptr_by_handle(handle);
    }
    return nullptr;
}

template<typename Component>
inline void world::remove(Entity e) const {
    getStorage<Component>()->erase(e);
}

template<typename Component>
inline bool world::tryRemove(Entity e) const {
    auto* storage = getStorage<Component>();
    if (storage->component.entityToHandle.get(e) != 0) {
        storage->erase(e);
        return true;
    }
    return false;
}

template<typename Component>
inline Component& world::getOrCreate(Entity e) const {
    auto* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        return storage->get_by_handle(handle);
    }
    return storage->emplace(e);
}


template<typename Component>
inline const Component& world::getOrDefault(Entity e) const {
    const auto* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    return storage->get_or_default_by_handle(handle);
}

template<typename ...Component>
inline void world::create(Entity* outEntities, uint32_t count) {
    create(outEntities, count);
    (assignBatch<Component>(outEntities, count), ...);
}

template<typename Component>
inline void world::registerComponent() {
    auto* storage = new ComponentStorage<Component>();
    registerComponent(&storage->component);
}

}