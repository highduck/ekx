#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/components/script.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/components/event_handler.hpp>

namespace ek {

inline ecs::entity create_node_2d(const std::string& name) {
    auto e = ecs::create<Node, Transform2D>();
    ecs::get<Node>(e).name = name;
    return e;
}

inline float2 global_to_local(ecs::entity e, const float2& position) {
    float2 res = position;
    auto it = e;
    while (it) {
        ecs::get<Transform2D>(it).matrix.transform_inverse(res, res);
        it = ecs::get<Node>(it).parent;
    }
    return res;
}

inline float2 global_to_parent(ecs::entity e, const float2& pos) {
    float2 res = pos;
    auto it = ecs::get<Node>(e).parent;
    while (it) {
        ecs::get<Transform2D>(it).matrix.transform_inverse(res, res);
        it = ecs::get<Node>(it).parent;
    }
    return res;
}

inline float2 local_to_global(ecs::entity e, const float2& pos) {
    float2 res = pos;
    auto it = e;
    while (it) {
        res = ecs::get<Transform2D>(it).matrix.transform(res);
        it = ecs::get<Node>(it).parent;
    }
    return res;
}

inline float2 local_to_local(ecs::entity src, ecs::entity dest, const float2& pos) {
    auto p = local_to_global(src, pos);
    return global_to_local(dest, p);
}

/*** events functions ***/

inline void dispatch_broadcast(ecs::entity e, const event_data& data) {
    if (e.has<event_handler_t>()) {
        e.get<event_handler_t>().emit(data);
    }
    eachChild(e, [&data](ecs::entity child) {
        dispatch_broadcast(child, data);
    });
}

inline void dispatch_bubble(ecs::entity e, const event_data& data) {
    auto it = e;
    while (it && it.valid()) {
        auto* eh = it.tryGet<event_handler_t>();
        if(eh) {
            eh->emit(data);
        }
        it = it.get<Node>().parent;
    }
}

inline void broadcast(ecs::entity e, const std::string& event) {
    dispatch_broadcast(e, {event, e, nullptr});
}

template<typename Payload>
inline void broadcast(ecs::entity e, const std::string& event, Payload payload) {
    dispatch_broadcast(e, {event, e, std::any{payload}});
}

template<>
inline void broadcast(ecs::entity e, const std::string& event, const char* payload) {
    dispatch_broadcast(e, {event, e, std::any{std::string{payload}}});
}

inline void notify_parents(ecs::entity e, const std::string& event, const std::string& payload = "") {
    dispatch_bubble(e, {event, e, std::any{payload}});
}

inline void set_gradient_quad(ecs::entity e, const rect_f& rc, argb32_t top, argb32_t bottom) {
    auto q = std::make_unique<Quad2D>();
    q->rect = rc;
    q->colors[0] = top;
    q->colors[1] = top;
    q->colors[2] = bottom;
    q->colors[3] = bottom;

    ecs::get_or_create<Display2D>(e).drawable = std::move(q);
}

inline void set_color_quad(ecs::entity e, const rect_f& rc, argb32_t color) {
    set_gradient_quad(e, rc, color, color);
}

template<typename Component>
inline ecs::entity find_first_ancestor(ecs::entity e) {
    auto it = ecs::get<Node>(e).parent;
    while (it) {
        if (ecs::has<Component>(it)) {
            return it;
        }
        it = ecs::get<Node>(it).parent;
    }
    return nullptr;
}

template<typename T>
inline T& get_drawable(ecs::entity e) {
    return *static_cast<T*>(ecs::get<Display2D>(e).drawable.get());
}

inline void set_text(ecs::entity e, const std::string& v) {
    auto& txt = get_drawable<Text2D>(e);
    txt.text = v;
}

}