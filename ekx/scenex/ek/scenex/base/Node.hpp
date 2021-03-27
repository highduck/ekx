#pragma once

#include <ecxx/ecxx.hpp>
#include <cassert>
#include <string>
#include <vector>

namespace ek {

struct NodeName {
    std::string name;
};

struct Node {
    enum Flags {
        Visible = 1,
        Touchable = 2,
        VisibleAndTouchable = Visible | Touchable,
        LayerMask = 0xFF00
    };

    ecs::entity sibling_next;
    ecs::entity sibling_prev;
    ecs::entity child_first;
    ecs::entity child_last;
    ecs::entity parent;

    uint16_t flags = LayerMask | VisibleAndTouchable;

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

    static int findDepth(ecs::entity e);

    static ecs::entity findLowerCommonAncestor(ecs::entity e1, ecs::entity e2);
};

template<typename Func>
inline void eachChild(ecs::entity e, Func func) {
    auto it = e.get<Node>().child_first;
    while (it) {
        auto temp = it;
        it = it.get<Node>().sibling_next;
        func(temp);
    }
}

template<typename Func>
inline void eachChildBackward(ecs::entity e, Func func) {
    auto it = e.get<Node>().child_last;
    while (it) {
        auto temp = it;
        it = it.get<Node>().sibling_prev;
        func(temp);
    }
}

ecs::entity getRoot(ecs::entity e);

inline ecs::entity getFirstChild(ecs::entity e) {
    assert(e.has<Node>());
    return e.get<Node>().child_first;
}

/**
    Delete all children and sub-children of entity `e`
    if `e` has Node component.
**/
void destroyChildren(ecs::entity e);

/**
    Returns true if entity is descendant of ancestor.
**/
bool isDescendant(ecs::entity e, ecs::entity ancestor);

/**
    Remove entity `e` from it's parent
    if `e` has Node component and is a child.
**/
void removeFromParent(ecs::entity e);

// child has node
// entity has node
// child hasn't parent
void appendStrict(ecs::entity e, ecs::entity child);

/**
    Add `child` to `entity` to the end.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
void append(ecs::entity e, ecs::entity child);

void prependStrict(ecs::entity e, ecs::entity child);

/**
    Add `child` to `entity` to the beginning.
    If `child` or `entity` have no Node component, it will be created.
    `child` will be removed from it's current parent.
**/
void prepend(ecs::entity e, ecs::entity child);

/**
    Remove all children of `entity`
    if `entity` has Node component and is a child.
**/
void removeChildren(ecs::entity e);

/**
    Insert `childAfter` next to the `entity`.
    Throws exception if `entity` has no parent.
    `childAfter` will be removed from it's current parent.
    If `childAfter` has not Node component, it will be added.
**/
void insertAfter(ecs::entity e, ecs::entity childAfter);

void insertBeforeStrict(ecs::entity e, ecs::entity childBefore);

/**
    Insert `childBefore` back to the `entity`.
    Throws exception if `entity` has no parent.
    `childBefore` will be removed from it's current parent.
    If `childBefore` has not Node component, it will be added.
**/
void insertBefore(ecs::entity e, ecs::entity child_before);

/**
   Number of children of `entity`.
   Returns 0 if `entity` has no Node component.

   Note: children will be counted in fast-traversing
   from the first to the last child of `entity`
**/
uint32_t countChildren(ecs::entity e);

// Destroy Entity (hierarchy way):
// - Remove Entity from parent
// - destroy all children
void destroyNode(ecs::entity e);

ecs::entity getChildAt(ecs::entity e, int index);

/** utility functions **/

inline void setName(ecs::entity e, const std::string& name) {
    e.get_or_create<NodeName>().name = name;
}

inline const std::string& getName(ecs::entity e) {
    return e.get_or_default<NodeName>().name;
}

inline bool isVisible(ecs::entity e) {
    return e.get_or_default<Node>().visible();
}

inline void setVisible(ecs::entity e, bool v) {
    e.get_or_create<Node>().setVisible(v);
}

inline bool isTouchable(ecs::entity e) {
    return e.get_or_default<Node>().touchable();
}

inline void setTouchable(ecs::entity e, bool v) {
    e.get_or_create<Node>().setTouchable(v);
}

/** components searching **/

// Recurse upwards until it finds a valid component of `Comp` type:
// 1) in the Entity `e`
// 2) any of its parents
//
// Returns `nullptr` if no component found
template<typename Comp>
Comp* findComponentInParent(ecs::entity e) {
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

ecs::entity find(ecs::entity e, const char* childName);

inline ecs::entity find(const ecs::entity e, const std::string& childName) {
    return find(e, childName.c_str());
}

ecs::entity findByPath(ecs::entity e, const std::vector<std::string>& path);

std::vector<ecs::entity> findMany(ecs::entity e, const std::vector<std::string>& names);

}