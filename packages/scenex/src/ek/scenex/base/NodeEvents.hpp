#pragma once

#include <ek/util/Signal.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/ds/String.hpp>
#include <utility>
#include "Node.hpp"

namespace ek {

union EventPayload {
    const void* pointer;
    const char* cstr;
    float f32;
    uint32_t u32;
    int64_t i64;
    int i32;
};

struct NodeEventData {
    const char* type;
    ecs::EntityApi source;
    EventPayload payload{nullptr};
    mutable bool processed = false;
};

struct NodeEventHandler {
    template<typename Fn>
    void on(const char* type, Fn&& listener) {
        signal.add(listener, type);
    }

    template<typename Fn>
    void once(const char* type, Fn&& listener) {
        signal.once(listener, type);
    }

    NodeEventData emit(const NodeEventData& event) {
        signal.emit_(event.type, event);
        return event;
    }

private:
    Signal<const NodeEventData&> signal;
};

ECX_TYPE(3, NodeEventHandler);

/*** events functions ***/

void dispatch_broadcast(ecs::EntityApi e, const NodeEventData& data);
void dispatch_bubble(ecs::EntityApi e, const NodeEventData& data);

inline void broadcast(ecs::EntityApi e, const char* event) {
    dispatch_broadcast(e, {event, e, {nullptr}});
}

inline void broadcast(ecs::EntityApi e, const char* event, float value) {
    NodeEventData data{event, e};
    data.payload.f32 = value;
    dispatch_broadcast(e, data);
}

inline void broadcast(ecs::EntityApi e, const char* event, int value) {
    NodeEventData data{event, e};
    data.payload.i32 = value;
    dispatch_broadcast(e, data);
}

inline void broadcast(ecs::EntityApi e, const char* event, const char* payload) {
    NodeEventData data{event, e, {payload}};
    dispatch_broadcast(e, data);
}

inline void notify_parents(ecs::EntityApi e, const char* event, const char* payload = "") {
    NodeEventData data{event, e, {payload}};
    dispatch_bubble(e, data);
}

}