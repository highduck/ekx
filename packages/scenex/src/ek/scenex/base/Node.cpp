#include "Node.hpp"

namespace ek {

ecs::EntityApi getChildAt(ecs::EntityApi e, int index) {
    int i = 0;
    auto it = e.get<Node>().child_first;
    while (it) {
        if (i == index) {
            return it;
        }
        ++i;
        it = it.get<Node>().sibling_next;
    }
    return nullptr;
}

void destroyChildren(ecs::EntityApi e) {
    auto* entityNode = e.tryGet<Node>();
    if (!entityNode) {
        return;
    }
    auto child = entityNode->child_first;
    while (child) {
        auto temp = child;
        auto& childNode = child.get<Node>();
        child = childNode.sibling_next;

        destroyChildren(temp);

        childNode.parent = nullptr;
        ecs::destroy(temp);
    }
    entityNode->child_first = nullptr;
    entityNode->child_last = nullptr;
}

bool isDescendant(ecs::EntityApi e, ecs::EntityApi ancestor) {
    if (!e.has<Node>() || !ancestor.has<Node>()) {
        return false;
    }

    while (e) {
        e = e.get<Node>().parent;
        if (e == ancestor) {
            return true;
        }
    }

    return false;
}

void removeFromParent(ecs::EntityApi e) {
    auto* entityNode = e.tryGet<Node>();
    if (!entityNode) {
        return;
    }

    auto par = entityNode->parent;
    if (!par) {
        return;
    }

    auto prev = entityNode->sibling_prev;
    auto next = entityNode->sibling_next;

    if (prev) {
        prev.get<Node>().sibling_next = next;
    } else {
        par.get<Node>().child_first = next;
    }

    if (next) {
        next.get<Node>().sibling_prev = prev;
    } else {
        par.get<Node>().child_last = prev;
    }

    entityNode->parent = nullptr;
    entityNode->sibling_next = nullptr;
    entityNode->sibling_prev = nullptr;
}

void appendStrict(ecs::EntityApi e, ecs::EntityApi child) {
    EK_ASSERT(e != child);
    EK_ASSERT(e.has<Node>());
    EK_ASSERT(child.has<Node>());
    EK_ASSERT(!child.get<Node>().parent);

    auto& entity_node = e.get<Node>();
    auto& child_node = child.get<Node>();
    auto tail = entity_node.child_last;
    if (tail) {
        tail.get<Node>().sibling_next = child;
        child_node.sibling_prev = tail;
        entity_node.child_last = child;
    } else {
        entity_node.child_first = child;
        entity_node.child_last = child;
    }
    child_node.parent = e;
}

void append(ecs::EntityApi e, ecs::EntityApi child) {
    e.get_or_create<Node>();
    if (child.get_or_create<Node>().parent) {
        removeFromParent(child);
    }
    appendStrict(e, child);
}

void prependStrict(ecs::EntityApi e, ecs::EntityApi child) {
    EK_ASSERT(e != child);
    EK_ASSERT(e.has<Node>());
    EK_ASSERT(child.has<Node>());
    EK_ASSERT(!child.get<Node>().parent);

    auto& entity_node = e.get<Node>();
    auto& child_node = child.get<Node>();
    auto head = entity_node.child_first;

    if (head) {
        child_node.sibling_next = head;
        head.get<Node>().sibling_prev = child;
        entity_node.child_first = child;
    } else {
        entity_node.child_first = child;
        entity_node.child_last = child;
    }
    child_node.parent = e;
}

void prepend(ecs::EntityApi e, ecs::EntityApi child) {
    e.get_or_create<Node>();
    if (child.get_or_create<Node>().parent) {
        removeFromParent(child);
    }
    prependStrict(e, child);
}

void removeChildren(ecs::EntityApi e) {
    auto* entityNode = e.tryGet<Node>();
    if (!entityNode) {
        return;
    }

    auto child = entityNode->child_first;
    while (child) {
        auto temp = child;
        child = child.get<Node>().sibling_next;
        auto& temp_node = temp.get<Node>();
        temp_node.parent = nullptr;
        temp_node.sibling_next = nullptr;
        temp_node.sibling_prev = nullptr;
    }
    entityNode->child_first = nullptr;
    entityNode->child_last = nullptr;
}

void insertAfter(ecs::EntityApi e, ecs::EntityApi childAfter) {
    auto& entityNode = e.get<Node>();
    EK_ASSERT(entityNode.parent);
    auto& childAfterNode = childAfter.get_or_create<Node>();
    removeFromParent(childAfter);
    auto next = entityNode.sibling_next;
    entityNode.sibling_next = childAfter;
    childAfterNode.sibling_prev = e;
    if (next) {
        next.get<Node>().sibling_prev = childAfter;
        childAfterNode.sibling_next = next;
    } else {
        entityNode.parent.get<Node>().child_last = childAfter;
    }
    childAfterNode.parent = entityNode.parent;
}

void insertBeforeStrict(ecs::EntityApi e, ecs::EntityApi childBefore) {
    EK_ASSERT(e.has<Node>());
    EK_ASSERT(childBefore.has<Node>());
    EK_ASSERT(!childBefore.get<Node>().parent);
    EK_ASSERT(e.get<Node>().parent);

    auto& entityNode = e.get<Node>();
    auto& childNode = childBefore.get<Node>();
    auto prev = entityNode.sibling_prev;
    entityNode.sibling_prev = childBefore;
    childNode.sibling_next = e;
    if (prev) {
        prev.get<Node>().sibling_next = childBefore;
        childNode.sibling_prev = prev;
    } else {
        entityNode.parent.get<Node>().child_first = childBefore;
    }
    childNode.parent = entityNode.parent;
}

void insertBefore(ecs::EntityApi e, ecs::EntityApi childBefore) {
    EK_ASSERT(e.get<Node>().parent);

    // TODO: a lot of places just require assign if not exists
    if (childBefore.get_or_create<Node>().parent) {
        removeFromParent(childBefore);
    }

    insertBeforeStrict(e, childBefore);
}

uint32_t countChildren(ecs::EntityApi e) {
    uint32_t num = 0u;
    auto* node = e.tryGet<Node>();
    if (node) {
        auto child = node->child_first;
        while (child) {
            ++num;
            child = child.get<Node>().sibling_next;
        }
    }
    return num;
}

void destroyNode(ecs::EntityApi e) {
    removeFromParent(e);
    destroyChildren(e);
    ecs::destroy(e);
}

ecs::EntityApi getRoot(ecs::EntityApi e) {
    EK_ASSERT(e.has<Node>());
    while (e && e.get<Node>().parent) {
        e = e.get<Node>().parent;
    }
    return e;
}

int Node::findDepth(ecs::EntityApi e) {
    int depth = 0;
    auto it = e.get<Node>().parent;
    while (it) {
        ++depth;
        it = it.get<Node>().parent;
    }
    return depth;
}

ecs::EntityApi Node::findLowerCommonAncestor(ecs::EntityApi e1, ecs::EntityApi e2) {
    auto depth1 = Node::findDepth(e1);
    auto depth2 = Node::findDepth(e2);
    auto it1 = e1;
    auto it2 = e2;
    while (depth1 > depth2) {
        it1 = it1.get<Node>().parent;
        --depth1;
    }
    while (depth2 < depth1) {
        it2 = it2.get<Node>().parent;
        --depth2;
    }
    while (it1 != it2) {
        it1 = it1.get<Node>().parent;
        it2 = it2.get<Node>().parent;
        if (!it1 || !it2) {
            return nullptr;
        }
    }
    return it1;
}


ecs::EntityApi find(ecs::EntityApi e, string_hash_t tag) {
    auto it = e.get<Node>().child_first;
    while (it) {
        const auto& n = it.get<Node>();
        if (n.tag == tag) {
            return it;
        }
        it = n.sibling_next;
    }
    return nullptr;
}

ecs::EntityApi findByPath(const ecs::EntityApi e, ...) {
    va_list argp;
    va_start(argp, e);

    auto it = e;
    string_hash_t p = va_arg(argp, string_hash_t);
    while(p != 0) {
        it = find(it, p);
        if (!it) {
            break;
        }
        p = va_arg(argp, string_hash_t);
    }
    va_end(argp);
    return it;
}

Array<ecs::EntityApi> findMany(const ecs::EntityApi e, ...) {
    va_list argp;
    va_start(argp, e);

    Array<ecs::EntityApi> entities;
    string_hash_t p = va_arg(argp, string_hash_t);
    while(p != 0) {
        auto f = find(e, p);
        if (f) {
            entities.push_back(f);
        }
        p = va_arg(argp, string_hash_t);
    }
    va_end(argp);
    return entities;
}

}