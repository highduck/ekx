#include "Node.hpp"

#include <stdarg.h>

namespace ek {

entity_t get_child_at(entity_t e, int index) {
    int i = 0;
    entity_t it = get_first_child(e);
    while (it.id) {
        if (i == index) {
            return it;
        }
        ++i;
        it = get_next_child(it);
    }
    return NULL_ENTITY;
}

void destroy_children(entity_t e) {
    Node* node = ecs::try_get<Node>(e);
    if (!node) {
        return;
    }
    entity_t child = node->child_first;
    while (child.id) {
        entity_t temp = child;
        Node& child_node = ecs::get<Node>(child);
        child = child_node.sibling_next;

        destroy_children(temp);

        child_node.parent = NULL_ENTITY;
        destroy_entity(temp);
    }
    node->child_first = NULL_ENTITY;
    node->child_last = NULL_ENTITY;
}

bool is_descendant(entity_t e, entity_t ancestor) {
    if (!ecs::has<Node>(e) || !ecs::has<Node>(ancestor)) {
        return false;
    }

    while (e.id) {
        e = get_parent(e);
        if (e == ancestor) {
            return true;
        }
    }

    return false;
}

void remove_from_parent(entity_t e) {
    Node* entityNode = ecs::try_get<Node>(e);
    if (!entityNode) {
        return;
    }

    entity_t par = entityNode->parent;
    if (!par.id) {
        return;
    }

    entity_t prev = entityNode->sibling_prev;
    entity_t next = entityNode->sibling_next;

    if (prev.id) {
        ecs::get<Node>(prev).sibling_next = next;
    } else {
        ecs::get<Node>(par).child_first = next;
    }

    if (next.id) {
        ecs::get<Node>(next).sibling_prev = prev;
    } else {
        ecs::get<Node>(par).child_last = prev;
    }

    entityNode->parent = NULL_ENTITY;
    entityNode->sibling_next = NULL_ENTITY;
    entityNode->sibling_prev = NULL_ENTITY;
}

void append_strict(entity_t e, entity_t child) {
    EK_ASSERT(e != child);
    EK_ASSERT(ecs::has<Node>(e));
    EK_ASSERT(ecs::has<Node>(child));
    EK_ASSERT(not get_parent(child).id);

    Node& entity_node = ecs::get<Node>(e);
    Node& child_node = ecs::get<Node>(child);
    entity_t tail = entity_node.child_last;
    if (tail.id) {
        ecs::get<Node>(tail).sibling_next = child;
        child_node.sibling_prev = tail;
        entity_node.child_last = child;
    } else {
        entity_node.child_first = child;
        entity_node.child_last = child;
    }
    child_node.parent = e;
}

void append(entity_t e, entity_t child) {
    ecs::add<Node>(e);
    if (get_parent(child).id) {
        remove_from_parent(child);
    }
    append_strict(e, child);
}

void prepend_strict(entity_t e, entity_t child) {
    EK_ASSERT(e.id != child.id);
    EK_ASSERT(ecs::has<Node>(e));
    EK_ASSERT(ecs::has<Node>(child));
    EK_ASSERT(!get_parent(child).id);

    Node& entity_node = ecs::get<Node>(e);
    Node& child_node = ecs::get<Node>(child);
    entity_t head = entity_node.child_first;

    if (head.id) {
        child_node.sibling_next = head;
        ecs::get<Node>(head).sibling_prev = child;
        entity_node.child_first = child;
    } else {
        entity_node.child_first = child;
        entity_node.child_last = child;
    }
    child_node.parent = e;
}

void prepend(entity_t e, entity_t child) {
    ecs::add<Node>(e);
    if (ecs::add<Node>(child).parent.id) {
        remove_from_parent(child);
    }
    prepend_strict(e, child);
}

void remove_children(entity_t e) {
    Node* entity_node = ecs::try_get<Node>(e);
    if (!entity_node) {
        return;
    }

    entity_t child = entity_node->child_first;
    while (child.id) {
        Node& child_node = ecs::get<Node>(child);
        child = child_node.sibling_next;
        child_node.parent = NULL_ENTITY;
        child_node.sibling_next = NULL_ENTITY;
        child_node.sibling_prev = NULL_ENTITY;
    }
    entity_node->child_first = NULL_ENTITY;
    entity_node->child_last = NULL_ENTITY;
}

void insert_after(entity_t e, entity_t child_after) {
    Node& entity_node = ecs::get<Node>(e);
    EK_ASSERT(entity_node.parent.id);
    Node& child_after_node = ecs::add<Node>(child_after);
    remove_from_parent(child_after);
    entity_t next = entity_node.sibling_next;
    entity_node.sibling_next = child_after;
    child_after_node.sibling_prev = e;
    if (next.id) {
        ecs::get<Node>(next).sibling_prev = child_after;
        child_after_node.sibling_next = next;
    } else {
        ecs::get<Node>(entity_node.parent).child_last = child_after;
    }
    child_after_node.parent = entity_node.parent;
}

void insert_before_strict(entity_t e, entity_t child_before) {
    EK_ASSERT(ecs::has<Node>(e));
    EK_ASSERT(ecs::has<Node>(child_before));
    EK_ASSERT(!ecs::get<Node>(child_before).parent.id);
    EK_ASSERT(ecs::get<Node>(e).parent.id);

    Node& entity_node = ecs::get<Node>(e);
    Node& child_node = ecs::get<Node>(child_before);
    entity_t prev = entity_node.sibling_prev;
    entity_node.sibling_prev = child_before;
    child_node.sibling_next = e;
    if (prev.id) {
        ecs::get<Node>(prev).sibling_next = child_before;
        child_node.sibling_prev = prev;
    } else {
        ecs::get<Node>(entity_node.parent).child_first = child_before;
    }
    child_node.parent = entity_node.parent;
}

void insert_before(entity_t e, entity_t child_before) {
    EK_ASSERT(get_parent(e).id);

    // TODO: a lot of places just require assign if not exists
    if (ecs::add<Node>(child_before).parent.id) {
        remove_from_parent(child_before);
    }

    insert_before_strict(e, child_before);
}

uint32_t count_children(entity_t e) {
    uint32_t num = 0u;
    Node* node = ecs::try_get<Node>(e);
    if (node) {
        entity_t child = node->child_first;
        while (child.id) {
            ++num;
            child = ecs::get<Node>(child).sibling_next;
        }
    }
    return num;
}

void destroy_node(entity_t e) {
    remove_from_parent(e);
    destroy_children(e);
    destroy_entity(e);
}

entity_t find_root(entity_t e) {
    entity_t root = NULL_ENTITY;
    while (is_entity(e)) {
        root = e;
        e = get_parent(e);
    }
    return root;
}

uint32_t get_node_depth(entity_t e) {
    uint32_t depth = 0;
    entity_t it = get_parent(e);
    while (it.id) {
        ++depth;
        it = get_parent(it);
    }
    return depth;
}

entity_t find_lower_common_ancestor(entity_t e1, entity_t e2) {
    uint32_t depth1 = get_node_depth(e1);
    uint32_t depth2 = get_node_depth(e2);
    entity_t it1 = e1;
    entity_t it2 = e2;
    while (depth1 > depth2) {
        it1 = get_parent(it1);
        --depth1;
    }
    while (depth2 < depth1) {
        it2 = get_parent(it2);
        --depth2;
    }
    while (it1.id != it2.id) {
        it1 = get_parent(it1);
        it2 = get_parent(it2);
        if (!it1.id || !it2.id) {
            return NULL_ENTITY;
        }
    }
    return it1;
}


entity_t find(entity_t e, string_hash_t tag) {
    entity_t it = ecs::get<Node>(e).child_first;
    while (it.id) {
        const Node& n = ecs::get<Node>(it);
        if (n.tag == tag) {
            return it;
        }
        it = n.sibling_next;
    }
    return NULL_ENTITY;
}

entity_t find_by_path(entity_t e, ...) {
    va_list argp;
    va_start(argp, e);

    string_hash_t tag = va_arg(argp, string_hash_t);
    while (tag) {
        e = find(e, tag);
        if (!e.id) {
            break;
        }
        tag = va_arg(argp, string_hash_t);
    }
    va_end(argp);
    return e;
}

uint32_t find_many(entity_t* out, const entity_t e, ...) {
    va_list argp;
    va_start(argp, e);

    uint32_t len = 0;
    string_hash_t tag = va_arg(argp, string_hash_t);
    while (tag) {
        entity_t f = find(e, tag);
        if (f.id) {
            out[len++] = f;
        }
        tag = va_arg(argp, string_hash_t);
    }
    va_end(argp);
    return len;
}

entity_t get_parent(entity_t e) {
    return ecs::get_or_default<Node>(e).parent;
}

void foreach_child(entity_t e, void(*callback)(entity_t child)) {
    entity_t it = get_first_child(e);
    while (it.id) {
        entity_t temp = it;
        it = get_next_child(it);
        callback(temp);
    }
}

void foreach_child_reverse(entity_t e, void(*callback)(entity_t child)) {
    entity_t it = get_last_child(e);
    while (it.id) {
        entity_t temp = it;
        it = get_prev_child(it);
        callback(temp);
    }
}

void* find_component_in_parent(ecx_component_type* type, entity_t e) {
    entity_t it = e;
    while (it.id) {
        void* c = get_component(type, it);
        if (c) {
            return c;
        }
        it = get_parent(it);
    }
    return NULL;
}

}