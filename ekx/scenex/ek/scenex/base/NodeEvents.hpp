#pragma once

#include <unordered_map>
#include <ek/util/signals.hpp>
#include <ecxx/ecxx.hpp>
#include <string>
#include <any>
#include <utility>
#include "Node.hpp"

namespace ek {

using NodeEventType = std::string;

struct NodeEventData {
    NodeEventType type;
    ecs::EntityApi source;
    std::any payload;
    mutable bool processed = false;
};

struct NodeEventHandler {
    using Callback = std::function<void(const NodeEventData&)>;
    using Signal = signal_t<const NodeEventData&>;

    Signal& getSignal(const NodeEventType& type) {
        return map_[type];
    }

    void on(const NodeEventType& type, Callback listener) {
        getSignal(type).add(std::move(listener));
    }

    void once(const NodeEventType& type, Callback listener) {
        getSignal(type).add_once(std::move(listener));
    }

//    void off(const NodeEventType& type, Callback listener) {
//        auto it = map_.find(type);
//        if (it != map_.end()) {
//            it->second.remove(listener);
//        }
//    }

    NodeEventData emit(const NodeEventData& event) {
        auto it = map_.find(event.type);
        if (it != map_.end()) {
            it->second.emit(event);
        }
        return event;
    }

private:
    std::unordered_map<NodeEventType, Signal> map_;
};

EK_DECLARE_TYPE(NodeEventHandler);

/*** events functions ***/

void dispatch_broadcast(ecs::EntityApi e, const NodeEventData& data);
void dispatch_bubble(ecs::EntityApi e, const NodeEventData& data);

inline void broadcast(ecs::EntityApi e, const std::string& event) {
    dispatch_broadcast(e, {event, e, nullptr});
}

template<typename Payload>
inline void broadcast(ecs::EntityApi e, const std::string& event, Payload payload) {
    dispatch_broadcast(e, {event, e, std::any{payload}});
}

template<>
inline void broadcast(ecs::EntityApi e, const std::string& event, const char* payload) {
    dispatch_broadcast(e, {event, e, std::any{std::string{payload}}});
}

inline void notify_parents(ecs::EntityApi e, const std::string& event, const std::string& payload = "") {
    dispatch_bubble(e, {event, e, std::any{payload}});
}

}