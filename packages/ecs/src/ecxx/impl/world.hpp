#pragma once

#include <ek/ds/Array.hpp>
#include <ek/assert.h>
#include <ek/sparse_array.h>
#include "../ecxx_fwd.hpp"

// for `std::is_empty`
#include <type_traits>

namespace ecs {

/** Component Managers **/

class World;

enum class StorageCommand {
    Emplace = 0,
    Erase = 1,
    Clear = 2,
    Shutdown = 3
};

// 256 + 74 = 330 bytes
// 128 * 330 = 42'240 kb
struct ComponentHeader {
    ek::Array<EntityIndex> handleToEntity; // dynamic (8 + 4 + 4 ~ 16 bytes)
    ek_sparse_array entityToHandle; // 256 bytes

    void* data; // 8 bytes

    ComponentHandle (* run)(StorageCommand cmd, ComponentHeader* component, EntityIndex entity); // 8 bytes

    uint32_t lockCounter; // 4 bytes

    // Debug info
    uint32_t storageElementSize = 0;
    const char* name = nullptr;
    ComponentTypeId typeId; // 2 bytes

    [[nodiscard]] inline unsigned count() const {
        return handleToEntity.size();
    }

    ComponentHeader(unsigned initialCapacity, void* manager) :
            handleToEntity(initialCapacity) {
        lockCounter = 0;
        data = manager;
        run = nullptr;
        handleToEntity.push_back(0);
    }

    static int compareBySize(const void* a, const void* b) {
        return (int)(*(const ComponentHeader**)a)->count() -
               (int)(*(const ComponentHeader**)b)->count();
    }
};

template<typename Component>
inline constexpr ComponentTypeId type() noexcept {
    return ek::TypeIndex<Component, World>::value;
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
    EntityGeneration generations[ENTITIES_MAX_COUNT]; // 65536 bytes

    //EntityLookup componentEntityToHandle[COMPONENTS_MAX_COUNT]; // 256 * 128 = 32768 bytes

    // per component, data manager
    ComponentHeader* components[COMPONENTS_MAX_COUNT]; // 8 * 128 = 1024 bytes

    // per component, maps entity to data handle
    ek_sparse_array entityToHandle;

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
    inline ComponentHandle get(EntityIndex e, ComponentTypeId cid) const {
        return ek_sparse_array_get(entityToHandle, ((uint32_t)cid << 16u) | e);
    }

    [[nodiscard]]
    inline ComponentHandle getOrCreate(EntityIndex e, ComponentTypeId cid) const {
        const auto handle = get(e, cid);
        if (handle == 0) {
            auto* component = getComponentHeader(cid);
            return component->run(StorageCommand::Emplace, component, e);
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

    void registerComponent(ComponentTypeId cid, ComponentHeader* component, const char* label) {
        EK_ASSERT(components[cid] == nullptr);
        EK_ASSERT_R2(component != nullptr);
        component->entityToHandle = ek_sparse_array_offset(entityToHandle, ((int)cid) << 16);
        component->name = label;
        component->typeId = cid;
        components[cid] = component;
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

    explicit ComponentStorage(unsigned initialCapacity) :
            component{initialCapacity, this},
            data(initialCapacity) {
        component.run = ComponentStorage<DataType>::s_run;
        component.storageElementSize = (uint32_t) sizeof(DataType);
        data.emplace_back();
    }

    template<typename ...Args>
    DataType& emplace(EntityIndex entity, Args&& ...args) {
        auto entityToHandle = component.entityToHandle;
        auto& handleToEntity = component.handleToEntity;
        const auto handle = component.count();
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(ek_sparse_array_get(entityToHandle, entity) == 0);

        ek_sparse_array_insert(entityToHandle, entity, handle);
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
        auto entityToHandle = component.entityToHandle;
        auto& handleToEntity = component.handleToEntity;
        const auto handle = component.count();
        EK_ASSERT(component.lockCounter == 0);
        EK_ASSERT(ek_sparse_array_get(entityToHandle, entity) == 0);

        ek_sparse_array_insert(entityToHandle, entity, handle);
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
        EK_ASSERT(ek_sparse_array_get(component.entityToHandle, entity) != 0);
        const auto backEntity = component.handleToEntity.back();
        if (entity != backEntity) {
            erase_from_middle(entity, backEntity);
        } else {
            erase_from_back(entity);
        }
    }

    void erase_from_middle(EntityIndex e, EntityIndex last) {
        const auto handle = ek_sparse_array_move_remove(component.entityToHandle, e, last);
        component.handleToEntity.set(handle, last);
        component.handleToEntity.pop_back();
        if constexpr (!EmptyData) {
            data.swapRemoveFromMiddle(handle);
        }
    }

    void erase_from_back(EntityIndex e) {
        ek_sparse_array_set(component.entityToHandle, e, 0);
        component.handleToEntity.pop_back();
        if constexpr (!EmptyData) {
            data.pop_back();
        }
    }

    inline DataType& get(EntityIndex e) const {
        if constexpr (EmptyData) {
            return const_cast<DataType&>(data.get(0));
        } else {
            const auto h = ek_sparse_array_get(component.entityToHandle, e);
            return const_cast<DataType&>(data.get(h));
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
            return data.begin();
        } else {
            return data.begin() + handle;
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
    constexpr static ComponentHandle s_run(StorageCommand cmd, ComponentHeader* hdr, EntityIndex e) {
        auto* storage = static_cast<ComponentStorage*>(hdr->data);
        switch(cmd) {
            case StorageCommand::Emplace:
                return storage->emplace_default(e);
            case StorageCommand::Erase:
                storage->erase(e);
                break;
            case StorageCommand::Clear:
                storage->data.reduceSize(1);
                break;
            case StorageCommand::Shutdown:
                storage->~ComponentStorage<DataType>();
                break;
        }
        return 0;
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
    const auto handle = ek_sparse_array_get(storage->component.entityToHandle, e);
    if (handle != 0) {
        auto& data = storage->get_by_handle(handle);
        data = {args...};
        return data;
    }
    return storage->emplace(e, args...);
}

template<typename Component>
inline bool World::has(EntityIndex e) const {
    return get(e, type<Component>()) != 0;
}

template<typename Component>
inline Component& World::get(EntityIndex e) const {
    return getStorage<Component>()->get(e);
}

template<typename Component>
inline Component* World::tryGet(EntityIndex e) const {
    const auto handle = get(e, type<Component>());
    if (handle != 0) {
        auto* storage = getStorage<Component>();
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
    const auto handle = get(e, type<Component>());
    if (handle != 0) {
        auto* storage = getStorage<Component>();
        storage->erase(e);
        return true;
    }
    return false;
}

template<typename Component>
inline Component& World::getOrCreate(EntityIndex e) const {
    const auto handle = get(e, type<Component>());
    auto* storage = getStorage<Component>();
    if (handle != 0) {
        return storage->get_by_handle(handle);
    }
    return storage->emplace(e);
}


template<typename Component>
inline const Component& World::getOrDefault(EntityIndex e) const {
    const auto handle = get(e, type<Component>());
    const auto* storage = getStorage<Component>();
    return storage->get_or_default_by_handle(handle);
}

template<typename ...Component>
inline void World::create(EntityIndex* outEntities, uint32_t count) {
    create(outEntities, count);
    (assignBatch<Component>(outEntities, count), ...);
}

template<typename Component>
inline void World::registerComponent(unsigned initialCapacity) {
    auto* storage = new ComponentStorage<Component>(initialCapacity);
    registerComponent(type<Component>(), &storage->component, ek::TypeName<Component>::value);
}

template<typename Component>
inline bool World::hasComponent() {
    const auto tid = type<Component>();
    return tid < COMPONENTS_MAX_COUNT && components[tid] != nullptr;
}

}