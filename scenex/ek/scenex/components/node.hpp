#pragma once

#include <ecxx/ecxx.hpp>

namespace ek {

struct node_t {
    ecs::entity parent;
    ecs::entity sibling_next;
    ecs::entity sibling_prev;
    ecs::entity child_first;
    ecs::entity child_last;
};

inline void world_enable_node(ecs::entity entity) {
    ecs::assign<node_t>(entity);
}

inline void world_disable_node(ecs::entity entity) {
    ecs::remove<node_t>(entity);
}

template<typename Func>
inline void each_child(ecs::entity e, Func func) {
    auto it = ecs::get<node_t>(e).child_first;
    while (it) {
        auto temp = it;
        it = ecs::get<node_t>(it).sibling_next;
        func(temp);
    }
}

template<typename Func>
inline void each_child_backward(ecs::entity e, Func func) {
    auto it = ecs::get<node_t>(e).child_last;
    while (it) {
        auto temp = it;
        it = ecs::get<node_t>(it).sibling_prev;
        func(temp);
    }
}

inline ecs::entity get_root(ecs::entity e) {
    assert(ecs::has<node_t>(e));
    while (e && ecs::get<node_t>(e).parent) {
        e = ecs::get<node_t>(e).parent;
    }
    return e;
}

inline ecs::entity try_get_first_child(ecs::entity e) {
    assert(ecs::has<node_t>(e));
    return ecs::get<node_t>(e).child_first;
}

/**
        Delete all children and sub-children of `entity`
        if `entity` has Node component.
    **/
inline void destroy_children(ecs::entity entity) {
    if (!ecs::has<node_t>(entity)) {
        return;
    }
    auto& entity_node = ecs::get<node_t>(entity);
    auto child = entity_node.child_first;
    while (child) {
        auto temp = child;
        child = ecs::get<node_t>(child).sibling_next;
        destroy_children(temp);

        ecs::get<node_t>(temp).parent = ecs::null;
        //world_disable_node(temp);
        ecs::destroy(temp);
    }
    entity_node.child_first = ecs::null;
    entity_node.child_last = ecs::null;
}

/**
    Returns true if entity is descendant of ancestor.
**/
inline bool is_descendant(ecs::entity entity, ecs::entity ancestor) {
    if (!ecs::has<node_t>(entity) || !ecs::has<node_t>(ancestor)) {
        return false;
    }

    while (entity) {
        entity = ecs::get<node_t>(entity).parent;
        if (entity == ancestor) {
            return true;
        }
    }

    return false;
}

/**
    Remove `entity` from it's parent
    if `entity` has Node component and is a child.
**/
inline void remove_from_parent(ecs::entity e) {
    if (!ecs::has<node_t>(e)) {
        return;
    }
    auto& entity_node = ecs::get<node_t>(e);
    auto par = entity_node.parent;
    if (!par) {
        return;
    }

    auto prev = entity_node.sibling_prev;
    auto next = entity_node.sibling_next;

    if (prev) {
        ecs::get<node_t>(prev).sibling_next = next;
    } else {
        ecs::get<node_t>(par).child_first = next;
    }

    if (next) {
        ecs::get<node_t>(next).sibling_prev = prev;
    } else {
        ecs::get<node_t>(par).child_last = prev;
    }

    entity_node.parent = ecs::null;
    entity_node.sibling_next = ecs::null;
    entity_node.sibling_prev = ecs::null;
}

// child has node
// entity has node
// child hasn't parent
inline void append_strict(ecs::entity e, ecs::entity child) {
    assert(e != child);
    assert(ecs::has<node_t>(e));
    assert(ecs::has<node_t>(child));
    assert(!ecs::get<node_t>(child).parent);

    auto& entity_node = ecs::get<node_t>(e);
    auto& child_node = ecs::get<node_t>(child);
    auto tail = entity_node.child_last;
    if (tail) {
        ecs::get<node_t>(tail).sibling_next = child;
        child_node.sibling_prev = tail;
        entity_node.child_last = child;
    } else {
        entity_node.child_first = child;
        entity_node.child_last = child;
    }
    child_node.parent = e;
}

/**
    Add `child` to `entity` to the end.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
inline void append(ecs::entity e, ecs::entity child) {
    if (!ecs::has<node_t>(e)) {
        world_enable_node(e);
    }
    if (!ecs::has<node_t>(child)) {
        world_enable_node(child);
    }
    if (ecs::get<node_t>(child).parent) {
        remove_from_parent(child);
    }

    append_strict(e, child);
}

inline void prepend_strict(ecs::entity entity, ecs::entity child) {
    assert(entity != child);
    assert(ecs::has<node_t>(entity));
    assert(ecs::has<node_t>(child));
    assert(!ecs::get<node_t>(child).parent);

    auto& entity_node = ecs::get<node_t>(entity);
    auto& child_node = ecs::get<node_t>(child);
    auto head = entity_node.child_first;

    if (head) {
        child_node.sibling_next = head;
        ecs::get<node_t>(head).sibling_prev = child;
        entity_node.child_first = child;
    } else {
        entity_node.child_first = child;
        entity_node.child_last = child;
    }
    child_node.parent = entity;
}

/**
        Add `child` to `entity` to the beginning.
        If `child` or `entity` have no Node component, it will be created.
        `child` will be removed from it's current parent.
    **/
inline void prepend(ecs::entity entity, ecs::entity child) {
    if (!ecs::has<node_t>(entity)) {
        world_enable_node(entity);
    }

    if (!ecs::has<node_t>(child)) {
        world_enable_node(child);
    }

    if (ecs::get<node_t>(child).parent) {
        remove_from_parent(child);
    }

    prepend_strict(entity, child);
}

/**
        Remove all children of `entity`
        if `entity` has Node component and is a child.
    **/
inline void remove_children(ecs::entity e) {
    if (!ecs::has<node_t>(e)) {
        return;
    }
    auto& entity_node = ecs::get<node_t>(e);
    auto child = entity_node.child_first;
    while (child) {
        auto temp = child;
        child = ecs::get<node_t>(child).sibling_next;
        auto& temp_node = ecs::get<node_t>(temp);
        temp_node.parent = ecs::null;
        temp_node.sibling_next = ecs::null;
        temp_node.sibling_prev = ecs::null;
    }
    entity_node.child_first = ecs::null;
    entity_node.child_last = ecs::null;
}

/**
		Insert `childAfter` next to the `entity`.
		Throws exception if `entity` has no parent.
		`childAfter` will be removed from it's current parent.
		If `childAfter` has not Node component, it will be added.
	**/
inline void insert_after(ecs::entity entity, ecs::entity child_after) {
    auto& entity_node = ecs::get<node_t>(entity);
    auto& child_after_node = ecs::get<node_t>(child_after);
    assert (entity_node.parent);
    if (!ecs::has<node_t>(child_after)) {
        world_enable_node(child_after);
    }
    remove_from_parent(child_after);
    auto next = entity_node.sibling_next;
    entity_node.sibling_next = child_after;
    child_after_node.sibling_prev = entity;
    if (next) {
        ecs::get<node_t>(next).sibling_prev = child_after;
        child_after_node.sibling_next = next;
    } else {
        ecs::get<node_t>(entity_node.parent).child_last = child_after;
    }
    child_after_node.parent = entity_node.parent;
}

inline void insert_before_strict(ecs::entity entity, ecs::entity child_before) {
    assert(ecs::has<node_t>(entity));
    assert(ecs::has<node_t>(child_before));
    assert(!ecs::get<node_t>(child_before).parent);
    assert(ecs::get<node_t>(entity).parent);

    auto& entity_node = ecs::get<node_t>(entity);
    auto& child_node = ecs::get<node_t>(child_before);
    //auto& child_node = get(child_before);
    auto prev = entity_node.sibling_prev;
    entity_node.sibling_prev = child_before;
    child_node.sibling_next = entity;
    if (prev) {
        ecs::get<node_t>(prev).sibling_next = child_before;
        child_node.sibling_prev = prev;
    } else {
        ecs::get<node_t>(entity_node.parent).child_first = child_before;
    }
    child_node.parent = entity_node.parent;
}

/**
    Insert `childBefore` back to the `entity`.
    Throws exception if `entity` has no parent.
    `childBefore` will be removed from it's current parent.
    If `childBefore` has not Node component, it will be added.
**/
inline void insert_before(ecs::entity e, ecs::entity child_before) {
    assert(ecs::get<node_t>(e).parent);

    if (!ecs::has<node_t>(child_before)) {
        world_enable_node(child_before);
    }
    if (ecs::get<node_t>(child_before).parent) {
        remove_from_parent(child_before);
    }

    insert_before_strict(e, child_before);
}

/**
       Number of children of `entity`.
       Returns 0 if `entity` has no Node component.

       Note: children will be counted in fast-traversing
       from the first to the last child of `entity`
   **/
inline uint32_t count_children(ecs::entity entity) {
    uint32_t num = 0u;
    if (ecs::has<node_t>(entity)) {
        auto child = ecs::get<node_t>(entity).child_first;
        while (child) {
            ++num;
            child = ecs::get<node_t>(child).sibling_next;
        }
    }
    return num;
}

inline void erase_node_component(ecs::entity e) {
    remove_from_parent(e);
    destroy_children(e);
    // TODO: remove_from_parent and destroy_children on component removing
    ecs::remove<node_t>(e);
}

// Destroy Entity (hierarchy way):
// - Remove Entity from parent
// - destroy all children
inline void destroy_node(ecs::entity e) {
    erase_node_component(e);
    ecs::destroy(e);
}

ecs::entity get_child_at(ecs::entity e, int index);

}


