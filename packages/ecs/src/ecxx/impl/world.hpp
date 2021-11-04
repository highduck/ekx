#pragma once

#include <ek/ds/SparseArray.hpp>
#include <ek/ds/Array.hpp>
#include <ek/assert.hpp>
#include <ek/util/Type.hpp>
#include "../ecxx_fwd.hpp"

// for `std::is_empty`
#include <type_traits>

namespace ecs {

/** Component Managers **/

struct ComponentRegistration {
    void* data; // 8 bytes

    ComponentHandle (* emplace)(void* component, EntityIndex entity); // 8 bytes

    void (* erase)(void* component, EntityIndex entity); // 8 bytes

    void (* clear)(void* component); // 8 bytes

    ComponentTypeId typeId; // 2 bytes
};

using EntityLookup = ek::SparseArray<EntityIndex, ComponentHandle, ENTITIES_MAX_COUNT>;

// 256 + 74 = 330 bytes
// 128 * 330 = 42'240 kb
struct ComponentHeader {
    EntityLookup entityToHandle; // 256 bytes

    ek::Array<EntityIndex> handleToEntity; // dynamic (8 + 4 + 4 ~ 16 bytes)

    void* data; // 8 bytes

    uint32_t storageElementSize = 0;
    // pointer to data array capacity for Profiling
    uint32_t* pDebugStorageCapacity = nullptr;

    // Type Name for Debugging
    const char* name = nullptr;

    ComponentHandle (* emplace)(ComponentHeader* component, EntityIndex entity); // 8 bytes

    void (* erase)(ComponentHeader* component, EntityIndex entity); // 8 bytes

    void (* clear)(ComponentHeader* component); // 8 bytes

    void (* shutdown)(void* manager); // 8 bytes

    uint32_t lockCounter; // 4 bytes

    ComponentTypeId typeId; // 2 bytes

    [[nodiscard]] inline unsigned count() const {
        return handleToEntity._size;
    }

    ComponentHeader(unsigned initialCapacity, ComponentTypeId typeId_, void* manager) : handleToEntity(
            initialCapacity) {
        typeId = typeId_;
        lockCounter = 0;
        data = manager;
        emplace = nullptr;
        erase = nullptr;
        clear = nullptr;
        shutdown = nullptr;
        entityToHandle.init();
        handleToEntity.push_back(0);
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
inline constexpr static ComponentTypeId type() noexcept {
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
class World {
public:
    // entity pool data, indices and generations
    // SIZE: 200 kb

    // entity indices
    //alignas(4096)
    EntityIndex entityPool[ENTITIES_MAX_COUNT]; // 2 * ENTITIES_MAX_COUNT = 131072 bytes

    // entity generations
    //alignas(4096)
    EntityGeneration generations[ENTITIES_MAX_COUNT]; // 65536 bytes

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
    EntityIndex next;

    // just zero entity (reserved null entity)
    EntityIndex zero;

    [[nodiscard]]
    inline EntityGeneration generation(EntityIndex e) const {
        return generations[e];
    }

    [[nodiscard]]
    inline bool isAllocated(EntityIndex e) const {
        return entityPool[e] == e;
    }

    /** lifecycle **/

    void initialize();

    void reset();

    void shutdown();

    /** entity pool **/

// entity create / destroy
    void create(EntityIndex* outEntities, uint32_t count);

    void destroy(const EntityIndex* entitiesToDestroy, uint32_t count);

    template<typename ...Component>
    void create(EntityIndex* outEntities, uint32_t count);

    // passport
    [[nodiscard]]
    bool check(EntityPassport passport) const;

    [[nodiscard]]
    inline bool isValid(EntityIndex e) const {
        return e && entityPool[e] == e;
    }

    /** components **/

    template<typename Component>
    [[nodiscard]] inline bool has(EntityIndex e) const;

    template<typename Component>
    [[nodiscard]] inline Component& get(EntityIndex e) const;

    template<typename Component>
    [[nodiscard]] inline Component* tryGet(EntityIndex e) const;

    template<typename Component, typename ...Args>
    inline Component& assign(EntityIndex e, Args&& ... args) const;

    template<typename Component, typename ...Args>
    inline void assignBatch(EntityIndex* entities, uint32_t count, Args&& ... args) const;

    template<typename Component, typename ...Args>
    inline Component& reassign(EntityIndex e, Args&& ... args) const;

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
    inline ComponentHandle get(EntityIndex e, ComponentTypeId componentId) const {
        return getComponentHeader(componentId)->entityToHandle.get(e);
    }

    [[nodiscard]]
    inline ComponentHandle getOrCreate(EntityIndex e, ComponentTypeId componentId) const {
        auto* component = getComponentHeader(componentId);
        const auto handle = component->entityToHandle.get(e);
        if (handle == 0) {
            return component->emplace(component, e);
        }
        return handle;
    }

    template<typename Component>
    inline void remove(EntityIndex e) const;

    template<typename Component>
    inline bool tryRemove(EntityIndex e) const;

    template<typename Component>
    inline Component& getOrCreate(EntityIndex e) const;

    template<typename Component>
    inline const Component& getOrDefault(EntityIndex e) const;

    inline void registerComponent(ComponentHeader* component) {
        components[component->typeId] = component;
    }

    template<typename Component>
    inline void registerComponent(unsigned initialCapacity = 1);

    template<typename Component>
    inline bool hasComponent();

private:
    void resetEntityPool();
};

/** Templated generic **/

template<typename DataType>
class ComponentStorage final {
public:
    ComponentHeader component;
    ek::Array<DataType> data;

    static constexpr bool EmptyData = std::is_empty_v<DataType>;

    explicit ComponentStorage(unsigned initialCapacity) : component{initialCapacity, type<DataType>(), this},
                                                          data(initialCapacity) {
        component.emplace = ComponentStorage<DataType>::s_emplace;
        component.erase = ComponentStorage<DataType>::s_erase;
        component.clear = ComponentStorage<DataType>::s_clear;
        component.shutdown = ComponentStorage<DataType>::s_shutdown;
        component.storageElementSize = (uint32_t) sizeof(DataType);
        component.pDebugStorageCapacity = &data._capacity;
        data.emplace_back();
    }

    template<typename ...Args>
    DataType& emplace(EntityIndex entity, Args&& ...args) {
        auto& entityToHandle = component.entityToHandle;
        auto& handleToEntity = component.handleToEntity;
        const auto handle = component.count();
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(entityToHandle.get(entity) == 0);

        entityToHandle.insert(entity, handle);
        handleToEntity.push_back(entity);
        if constexpr (EmptyData) {
            return data.get(0);
        } else {
            if constexpr (std::is_aggregate_v<DataType>) {
                return data.emplace_back(DataType{args...});
            } else {
                return data.emplace_back(args...);
            }
        }
    }

    ComponentHandle emplace_default(EntityIndex entity) {
        auto& entityToHandle = component.entityToHandle;
        auto& handleToEntity = component.handleToEntity;
        const auto handle = component.count();
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(entityToHandle.get(entity) == 0);

        entityToHandle.insert(entity, handle);
        handleToEntity.push_back(entity);
        if constexpr (EmptyData) {
            return 0;
        } else {
            data.emplace_back();
            return handle;
        }
    }

    void erase(EntityIndex entity) {
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(component.entityToHandle.get(entity) != 0);
        const auto backEntity = component.handleToEntity.back();
        if (entity != backEntity) {
            erase_from_middle(entity, backEntity);
        } else {
            erase_from_back(entity);
        }
    }

    void erase_from_middle(EntityIndex e, EntityIndex last) {
        const auto handle = component.entityToHandle.moveRemove(e, last);
        component.handleToEntity.set(handle, last);
        component.handleToEntity.pop_back();
        if constexpr (!EmptyData) {
            data.swapRemoveFromMiddle(handle);
        }
    }

    void erase_from_back(EntityIndex e) {
        component.entityToHandle.set(e, 0);
        component.handleToEntity.pop_back();
        if constexpr (!EmptyData) {
            data.pop_back();
        }
    }

    inline DataType& get(EntityIndex e) const {
        if constexpr (EmptyData) {
            return const_cast<DataType&>(data.get(0));
        } else {
            return const_cast<DataType&>(data.get(component.entityToHandle.get(e)));
        }
    }

    inline DataType& get_by_handle(ComponentHandle handle) const {
        if constexpr (EmptyData) {
            return get_data(0u);
        } else {
            return get_data(handle);
        }
    }

    inline DataType* get_ptr_by_handle(ComponentHandle handle) {
        if constexpr (EmptyData) {
            return data._data;
        } else {
            return data._data + handle;
        }
    }

    DataType& get_or_default_by_handle(ComponentHandle handle) const {
        if constexpr (EmptyData) {
            return get_data(0u);
        } else {
            return get_data(handle);
        }
    }

    /** Methods used for dynamic operations instead of using virtual calls **/
    static ComponentHandle s_emplace(ComponentHeader* hdr, EntityIndex e) {
        return static_cast<ComponentStorage*>(hdr->data)->emplace_default(e);
    }

    static void s_erase(ComponentHeader* hdr, EntityIndex entity) {
        static_cast<ComponentStorage*>(hdr->data)->erase(entity);
    }

    static void s_clear(ComponentHeader* hdr) {
        static_cast<ComponentStorage*>(hdr->data)->data.reduceSize(1);
    }

    static void s_shutdown(void* manager) {
        static_cast<ComponentStorage<DataType>*>(manager)->~ComponentStorage<DataType>();
    }

    inline DataType& get_data(ComponentHandle i) const {
        return const_cast<DataType&>(data.get(i));
    }

    // get data by packed index
    inline DataType& get_data_by_index(ComponentHandle i) const {
        if constexpr (EmptyData) {
            return const_cast<DataType&>(data.get(0));
        } else {
            return const_cast<DataType&>(data.get(i));
        }
    }
};

template<typename Component>
inline ComponentStorage<Component>* World::getStorage() const {
    auto* component = components[type<Component>()];
    EK_ASSERT_R2(component != nullptr);
    return static_cast<ComponentStorage<Component>*>(component->data);
}

template<typename Component, typename ...Args>
inline Component& World::assign(EntityIndex e, Args&& ... args) const {
    return getStorage<Component>()->emplace(e, args...);
}

template<typename Component, typename ...Args>
inline void World::assignBatch(EntityIndex* entities, uint32_t count, Args&& ... args) const {
    auto* storage = getStorage<Component>();
    for (uint32_t i = 0; i < count; ++i) {
        storage->emplace(entities[i], args...);
    }
}

template<typename Component, typename ...Args>
inline Component& World::reassign(EntityIndex e, Args&& ... args) const {
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
inline bool World::has(EntityIndex e) const {
    const auto* component = getComponentHeader(type<Component>());
    const auto& entityToHandle = component->entityToHandle;
    const auto handle = entityToHandle.get(e);
    return handle != 0;
}

template<typename Component>
inline Component& World::get(EntityIndex e) const {
    return getStorage<Component>()->get(e);
}

template<typename Component>
inline Component* World::tryGet(EntityIndex e) const {
    auto* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        return storage->get_ptr_by_handle(handle);
    }
    return nullptr;
}

template<typename Component>
inline void World::remove(EntityIndex e) const {
    getStorage<Component>()->erase(e);
}

template<typename Component>
inline bool World::tryRemove(EntityIndex e) const {
    auto* storage = getStorage<Component>();
    if (storage->component.entityToHandle.get(e) != 0) {
        storage->erase(e);
        return true;
    }
    return false;
}

template<typename Component>
inline Component& World::getOrCreate(EntityIndex e) const {
    auto* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    if (handle != 0) {
        return storage->get_by_handle(handle);
    }
    return storage->emplace(e);
}


template<typename Component>
inline const Component& World::getOrDefault(EntityIndex e) const {
    const auto* storage = getStorage<Component>();
    const auto handle = storage->component.entityToHandle.get(e);
    return storage->get_or_default_by_handle(handle);
}

template<typename ...Component>
inline void World::create(EntityIndex* outEntities, uint32_t count) {
    create(outEntities, count);
    (assignBatch<Component>(outEntities, count), ...);
}

template<typename Component>
inline void World::registerComponent(unsigned initialCapacity) {
    const char* label = ek::Type<Component>::Data.label;
    {
        auto* storage = new ComponentStorage<Component>(initialCapacity);
        storage->component.name = label;
        registerComponent(&storage->component);
    }
}

template<typename Component>
inline bool World::hasComponent() {
    const auto tid = type<Component>();
    return tid < COMPONENTS_MAX_COUNT && components[tid] != nullptr;
}

}