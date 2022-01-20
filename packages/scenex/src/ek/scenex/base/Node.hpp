#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/assert.h>
#include <ek/ds/String.hpp>
#include <ek/ds/Array.hpp>

namespace ek {

struct Node {
    enum Flags {
        Visible = 1,
        Touchable = 2,
        VisibleAndTouchable = Visible | Touchable,
        LayerMask = 0xFF00
    };

    ecs::EntityApi sibling_next;
    ecs::EntityApi sibling_prev;
    ecs::EntityApi child_first;
    ecs::EntityApi child_last;
    ecs::EntityApi parent;

    uint16_t flags = LayerMask | VisibleAndTouchable;
    string_hash_t tag;

    [[nodiscard]] bool visible() const {
        return (flags & Visible) != 0;
    }

    [[nodiscard]] bool touchable() const {
        return (flags & Touchable) != 0;
    }

    void setVisible(bool v) {
        flags = v ? (flags | Visible) : (flags & ~Visible);
    }

    void setTouchable(bool v) {
        flags = v ? (flags | Touchable) : (flags & ~Touchable);
    }

    [[nodiscard]] uint16_t layersMask() const {
        return (flags & LayerMask) >> 8;
    }

    void setLayersMask(uint16_t mask) {
        flags |= (flags & ~LayerMask) | ((mask << 8) & LayerMask);
    }

    static int findDepth(ecs::EntityApi e);

    static ecs::EntityApi findLowerCommonAncestor(ecs::EntityApi e1, ecs::EntityApi e2);
};


template<typename Func>
inline void eachChild(ecs::EntityApi e, Func func) {
    auto it = e.get<Node>().child_first;
    while (it) {
        auto temp = it;
        it = it.get<Node>().sibling_next;
        func(temp);
    }
}

template<typename Func>
inline void eachChildBackward(ecs::EntityApi e, Func func) {
    auto it = e.get<Node>().child_last;
    while (it) {
        auto temp = it;
        it = it.get<Node>().sibling_prev;
        func(temp);
    }
}

ecs::EntityApi getRoot(ecs::EntityApi e);

inline ecs::EntityApi getFirstChild(ecs::EntityApi e) {
    EK_ASSERT(e.has<Node>());
    return e.get<Node>().child_first;
}

/**
    Delete all children and sub-children of entity `e`
    if `e` has Node component.
**/
void destroyChildren(ecs::EntityApi e);

/**
    Returns true if entity is descendant of ancestor.
**/
bool isDescendant(ecs::EntityApi e, ecs::EntityApi ancestor);

/**
    Remove entity `e` from it's parent
    if `e` has Node component and is a child.
**/
void removeFromParent(ecs::EntityApi e);

// child has node
// entity has node
// child hasn't parent
void appendStrict(ecs::EntityApi e, ecs::EntityApi child);

/**
    Add `child` to `entity` to the end.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
void append(ecs::EntityApi e, ecs::EntityApi child);

void prependStrict(ecs::EntityApi e, ecs::EntityApi child);

/**
    Add `child` to `entity` to the beginning.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
void prepend(ecs::EntityApi e, ecs::EntityApi child);

/**
    Remove all children of `entity`
    if `entity` has Node component and is a child.
**/
void removeChildren(ecs::EntityApi e);

/**
    Insert `childAfter` next to the `entity`.
    Throws exception if `entity` has no parent.
    `childAfter` will be removed from it's current parent.
    If `childAfter` has not Node component, it will be added.
**/
void insertAfter(ecs::EntityApi e, ecs::EntityApi childAfter);

void insertBeforeStrict(ecs::EntityApi e, ecs::EntityApi childBefore);

/**
    Insert `childBefore` back to the `entity`.
    Throws exception if `entity` has no parent.
    `childBefore` will be removed from it's current parent.
    If `childBefore` has not Node component, it will be added.
**/
void insertBefore(ecs::EntityApi e, ecs::EntityApi child_before);

/**
   Number of children of `entity`.
   Returns 0 if `entity` has no Node component.

   Note: children will be counted in fast-traversing
   from the first to the last child of `entity`
**/
uint32_t countChildren(ecs::EntityApi e);

// Destroy Entity (hierarchy way):
// - Remove Entity from parent
// - destroy all children
void destroyNode(ecs::EntityApi e);

ecs::EntityApi getChildAt(ecs::EntityApi e, int index);

/** utility functions **/

inline void set_tag(ecs::EntityApi e, string_hash_t tag) {
    e.get_or_create<Node>().tag = tag;
}

inline string_hash_t get_tag(ecs::EntityApi e) {
    return e.get_or_default<Node>().tag;
}

inline bool isVisible(ecs::EntityApi e) {
    return e.get_or_default<Node>().visible();
}

inline void setVisible(ecs::EntityApi e, bool v) {
    e.get_or_create<Node>().setVisible(v);
}

inline bool isTouchable(ecs::EntityApi e) {
    return e.get_or_default<Node>().touchable();
}

inline void setTouchable(ecs::EntityApi e, bool v) {
    e.get_or_create<Node>().setTouchable(v);
}

/** components searching **/

// Recurse upwards until it finds a valid component of `Comp` type:
// 1) in the Entity `e`
// 2) any of its parents
//
// Returns `nullptr` if no component found
template<typename Comp>
Comp* findComponentInParent(ecs::EntityApi e) {
    auto it = e;
    while (it) {
        auto* c = it.tryGet<Comp>();
        if (c) {
            return c;
        }
        it = it.get<Node>().parent;
    }
    return nullptr;
}

/** search functions **/

ecs::EntityApi find(ecs::EntityApi e, string_hash_t tag);

ecs::EntityApi findByPath(ecs::EntityApi e, ...);

Array <ecs::EntityApi> findMany(ecs::EntityApi e, ...);

}