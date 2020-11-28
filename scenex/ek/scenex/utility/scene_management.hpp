#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/base/Script.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/base/NodeEvents.hpp>

namespace ek {

inline ecs::entity create_node_2d(const std::string& name) {
    auto e = ecs::create<Node, Transform2D>();
    ecs::get<Node>(e).name = name;
    return e;
}

/*** events functions ***/

inline void dispatch_broadcast(ecs::entity e, const NodeEventData& data) {
    if (e.has<NodeEventHandler>()) {
        e.get<NodeEventHandler>().emit(data);
    }
    eachChild(e, [&data](ecs::entity child) {
        dispatch_broadcast(child, data);
    });
}

inline void dispatch_bubble(ecs::entity e, const NodeEventData& data) {
    auto it = e;
    while (it && it.valid()) {
        auto* eh = it.tryGet<NodeEventHandler>();
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

}