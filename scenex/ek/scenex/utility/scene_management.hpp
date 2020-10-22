#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/components/node.hpp>
#include <ek/scenex/components/transform_2d.hpp>
#include <ek/scenex/components/script.hpp>
#include <ek/scenex/components/name.hpp>
#include <ek/scenex/components/display_2d.hpp>
#include <ek/scenex/components/event_handler.hpp>

namespace ek {

inline ecs::entity create_node_2d(const std::string& name) {
    auto e = ecs::create<node_t, transform_2d, name_t>();
    ecs::get<name_t>(e).name = name;
    return e;
}

inline void set_alpha(ecs::entity e, float alpha) {
    ecs::get_or_create<transform_2d>(e).set_alpha(alpha);
}

inline void set_color_multiplier(ecs::entity e, argb32_t color_multiplier) {
    ecs::get_or_create<transform_2d>(e).color_multiplier = color_multiplier;
}

inline void set_color_offset(ecs::entity e, argb32_t color_offset) {
    ecs::get_or_create<transform_2d>(e).color_offset = color_offset;
}

inline void set_position(ecs::entity e, const float2& pos) {
    ecs::get_or_create<transform_2d>(e).matrix.position(pos);
}

inline float2 get_position(const ecs::entity e) {
    return ecs::get_or_default<transform_2d>(e).matrix.position();
}

inline void set_rotation(ecs::entity e, float radians) {
    ecs::get_or_create<transform_2d>(e).rotation(radians);
}

inline float get_rotation(const ecs::entity e) {
    return ecs::get_or_default<transform_2d>(e).rotation();
}

inline void set_scale(ecs::entity e, const float2& sc) {
    ecs::get_or_create<transform_2d>(e).scale = sc;
}

inline void set_scale(ecs::entity e, float xy) {
    ecs::get_or_create<transform_2d>(e).scale = {xy, xy};
}

inline float2 get_scale(ecs::entity e) {
    return ecs::get_or_default<transform_2d>(e).scale;
}

inline void set_name(ecs::entity e, const std::string& name) {
    ecs::get_or_create<name_t>(e).name = name;
}

inline const std::string& get_name(ecs::entity e) {
    return ecs::get_or_default<name_t>(e).name;
}

inline bool is_visible(ecs::entity e) {
    return ecs::get_or_default<node_state_t>(e).visible;
}

inline void set_visible(ecs::entity e, bool v) {
    ecs::get_or_create<node_state_t>(e).visible = v;
}

inline bool is_touchable(ecs::entity e) {
    return ecs::get_or_default<node_state_t>(e).touchable;
}

inline void set_touchable(ecs::entity e, bool v) {
    ecs::get_or_create<node_state_t>(e).touchable = v;
}

template<typename S>
inline S& assign_script(ecs::entity e) {
    if (!ecs::has<script_holder>(e)) {
        ecs::assign<script_holder>(e);
    }
    auto& holder = ecs::get<script_holder>(e);
    auto& script = holder.list.emplace_back(std::make_unique<S>());
    script->link_to_entity(e);
    script->start();
    return *static_cast<S*>(script.get());
}

template<typename S>
inline S& get_script(ecs::entity e) {
    const auto interest_type_id = type_index<S, script_cpp_base>::value;
    auto& h = ecs::get<script_holder>(e);
    for (auto& script : h.list) {
        if (script && script->get_type_id() == interest_type_id) {
            return static_cast<S&>(*script);
        }
    }
    abort();
}

inline float2 global_to_local(ecs::entity e, const float2& position) {
    float2 res = position;
    auto it = e;
    while (it) {
        ecs::get<transform_2d>(it).matrix.transform_inverse(res, &res);
        it = ecs::get<node_t>(it).parent;
    }
    return res;
}

inline float2 global_to_parent(ecs::entity e, const float2& pos) {
    float2 res = pos;
    auto it = ecs::get<node_t>(e).parent;
    while (it) {
        ecs::get<transform_2d>(it).matrix.transform_inverse(res, &res);
        it = ecs::get<node_t>(it).parent;
    }
    return res;
}

inline float2 local_to_global(ecs::entity e, const float2& pos) {
    float2 res = pos;
    auto it = e;
    while (it) {
        res = ecs::get<transform_2d>(it).matrix.transform(res);
        it = ecs::get<node_t>(it).parent;
    }
    return res;
}

inline float2 local_to_local(ecs::entity src, ecs::entity dest, const float2& pos) {
    auto p = local_to_global(src, pos);
    return global_to_local(dest, p);
}

/*** events functions ***/

inline void dispatch_broadcast(ecs::entity e, const event_data& data) {
    if (ecs::has<event_handler_t>(e)) {
        ecs::get<event_handler_t>(e).emit(data);
    }
    each_child(e, [&data](ecs::entity child) {
        dispatch_broadcast(child, data);
    });
}

inline void dispatch_bubble(ecs::entity e, const event_data& data) {
    auto it = e;
    while (it && ecs::valid(it)) {
        if (ecs::has<event_handler_t>(it)) {
            ecs::get<event_handler_t>(it).emit(data);
        }
        it = ecs::get<node_t>(it).parent;
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


/*** finds ***/

inline ecs::entity find(const ecs::entity e, const char* child_name) {
    auto it = ecs::get<node_t>(e).child_first;
    while (it) {
        if (ecs::get_or_default<name_t>(it).name == child_name) {
            return it;
        }
        it = ecs::get<node_t>(it).sibling_next;
    }
    return ecs::null;
}

inline ecs::entity find(const ecs::entity e, const std::string& child_name) {
    return find(e, child_name.c_str());
}

inline ecs::entity find_path(const ecs::entity e, const std::vector<std::string>& path) {
    auto it = e;
    for (const auto& p : path) {
        it = find(it, p);
        if (!it) {
            return ecs::null;
        }
    }
    return it;
}

inline std::vector<ecs::entity> find_array(const ecs::entity e, const std::vector<std::string>& names) {
    std::vector<ecs::entity> entities;
    for (const auto& name : names) {
        auto f = find(e, name);
        if (f) {
            entities.push_back(f);
        }
    }
    return entities;
}

inline void set_gradient_quad(ecs::entity e, const rect_f& rc, argb32_t top, argb32_t bottom) {
    auto q = std::make_unique<drawable_quad>();
    q->rect = rc;
    q->colors[0] = top;
    q->colors[1] = top;
    q->colors[2] = bottom;
    q->colors[3] = bottom;

    ecs::get_or_create<display_2d>(e).drawable = std::move(q);
}

inline void set_color_quad(ecs::entity e, const rect_f& rc, argb32_t color) {
    set_gradient_quad(e, rc, color, color);
}

template<typename Component>
inline ecs::entity find_first_ancestor(ecs::entity e) {
    auto it = ecs::get<node_t>(e).parent;
    while (it) {
        if (ecs::has<Component>(it)) {
            return it;
        }
        it = ecs::get<node_t>(it).parent;
    }
    return ecs::null;
}

template<typename T>
inline T& get_drawable(ecs::entity e) {
    return *static_cast<T*>(ecs::get<display_2d>(e).drawable.get());
}

inline void set_text(ecs::entity e, const std::string& v) {
    auto& txt = get_drawable<drawable_text>(e);
    txt.text = v;
}

}