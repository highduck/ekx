`entity` - is stable `int16` identifier used as index. `0` index used as invalid null-entity.
`passport` - is mix of `entity` index and cyclic version number (generation) to store and invalidate logical references to entity.
`component_type` - is data manager which control association of entity with some logical component and it's data.
`component_handle` - is index used to access component's data arrays.

## Lifecycle

- user initialize ecs system
- user creates new entity (no `on_entity_created` callback)
- add new component to entity: component type `ctor` is called.
- remove component from entity: component type `dtor` is called.
- user destroy entity
    - system clears entity from all components
    - `on_entity_destroy` callbacks fired
- user shutdown ecs system
    - system destroys all component data (`dtor` calls for each created data)
    - call `shutdown` callback for each component type, here user should terminate temporary communication managers between ecs components and external systems.
- user shutdown external systems

