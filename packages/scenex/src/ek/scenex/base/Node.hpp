#pragma once

#include <ecx/ecx.hpp>
#include <ek/assert.h>
#include <ek/hash.h>
#include <ek/ds/PodArray.hpp>

enum node_flags_t {
    NODE_HIDDEN = 1,
    NODE_UNTOUCHABLE = 2
};

namespace ek {

struct Node {

    entity_t parent = NULL_ENTITY;
    entity_t child_first = NULL_ENTITY;
    entity_t sibling_next = NULL_ENTITY;
    entity_t child_last = NULL_ENTITY;
    entity_t sibling_prev = NULL_ENTITY;

    uint32_t flags = 0;
    string_hash_t tag = 0;
};

uint32_t get_node_depth(entity_t e);

entity_t find_root(entity_t e);

entity_t find_lower_common_ancestor(entity_t e1, entity_t e2);

inline entity_t get_first_child(entity_t e) {
    return ecs::get<Node>(e).child_first;
}

inline entity_t get_next_child(entity_t e) {
    return ecs::get<Node>(e).sibling_next;
}

inline entity_t get_last_child(entity_t e) {
    return ecs::get<Node>(e).child_last;
}

inline entity_t get_prev_child(entity_t e) {
    return ecs::get<Node>(e).sibling_prev;
}

/**
    Delete all children and sub-children of entity `e`
    if `e` has Node component.
**/
void destroy_children(entity_t e);

/**
    Returns true if entity is descendant of ancestor.
**/
bool is_descendant(entity_t e, entity_t ancestor);

/**
    Remove entity `e` from it's parent
    if `e` has Node component and is a child.
**/
void remove_from_parent(entity_t e);

// child has node
// entity has node
// child hasn't parent
void append_strict(entity_t e, entity_t child);

/**
    Add `child` to `entity` to the end.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
void append(entity_t e, entity_t child);

void prepend_strict(entity_t e, entity_t child);

/**
    Add `child` to `entity` to the beginning.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
void prepend(entity_t e, entity_t child);

/**
    Remove all children of `entity`
    if `entity` has Node component and is a child.
**/
void remove_children(entity_t e);

/**
    Insert `childAfter` next to the `entity`.
    Throws exception if `entity` has no parent.
    `childAfter` will be removed from it's current parent.
    If `childAfter` has not Node component, it will be added.
**/
void insert_after(entity_t e, entity_t child_after);

void insert_before_strict(entity_t e, entity_t child_before);

/**
    Insert `childBefore` back to the `entity`.
    Throws exception if `entity` has no parent.
    `childBefore` will be removed from it's current parent.
    If `childBefore` has not Node component, it will be added.
**/
void insert_before(entity_t e, entity_t child_before);

/**
   Number of children of `entity`.
   Returns 0 if `entity` has no Node component.

   Note: children will be counted in fast-traversing
   from the first to the last child of `entity`
**/
uint32_t count_children(entity_t e);

// Destroy Entity (hierarchy way):
// - Remove Entity from parent
// - destroy all children
void destroy_node(entity_t e);

entity_t get_child_at(entity_t e, int index);

/** utility functions **/

inline void set_tag(entity_t e, string_hash_t tag) {
    ecs::add<Node>(e).tag = tag;
}

inline string_hash_t get_tag(entity_t e) {
    return ecs::get_or_default<Node>(e).tag;
}

inline bool is_visible(entity_t e) {
    Node* node = ecs::try_get<Node>(e);
    return node && !(node->flags & NODE_HIDDEN);
}

inline void set_visible(entity_t e, bool v) {
    Node* node = ecs::try_get<Node>(e);
    node->flags = v ? (node->flags & ~NODE_HIDDEN) : (node->flags | NODE_HIDDEN);
}

inline bool is_touchable(entity_t e) {
    Node* node = ecs::try_get<Node>(e);
    return node && !(node->flags & NODE_UNTOUCHABLE);
}

inline void set_touchable(entity_t e, bool v) {
    Node* node = ecs::try_get<Node>(e);
    node->flags = v ? (node->flags & ~NODE_UNTOUCHABLE) : (node->flags | NODE_UNTOUCHABLE);
}

/** components searching **/

// Recurse upwards until it finds a valid component of `Comp` type:
// 1) in the Entity `e`
// 2) any of its parents
// Returns `NULL` if no component found
void* find_component_in_parent(ecx_component_type* type, entity_t e);

void foreach_child(entity_t e, void(*callback)(entity_t child));

void foreach_child_reverse(entity_t e, void(*callback)(entity_t child));

/** search functions **/

entity_t find(entity_t e, string_hash_t tag);

entity_t find_by_path(entity_t e, ...);

uint32_t find_many(entity_t* out, entity_t e, ...);

entity_t get_parent(entity_t e);

}
