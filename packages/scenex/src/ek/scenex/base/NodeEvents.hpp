#pragma once

#include <utility>
#include <ek/util/Signal.hpp>
#include <ecx/ecx.hpp>
#include <ek/ds/String.hpp>
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
    string_hash_t type;
    entity_t source;
    EventPayload payload{nullptr};
    mutable entity_t receiver;
    mutable bool processed = false;
};

struct NodeEventHandler {
    template<typename Fn>
    void on(string_hash_t type, Fn&& listener) {
        signal.add(listener, type);
    }

    template<typename Fn>
    void once(string_hash_t type, Fn&& listener) {
        signal.once(listener, type);
    }

    NodeEventData emit(const NodeEventData& event) {
        signal.emit_(event.type, event);
        return event;
    }

    Signal<const NodeEventData&> signal;
};



/*** events functions ***/

void dispatch_broadcast(entity_t e, const NodeEventData& data);
void dispatch_bubble(entity_t e, const NodeEventData& data);

inline void broadcast(entity_t e, string_hash_t event) {
    dispatch_broadcast(e, {event, e, {nullptr}});
}

inline void broadcast(entity_t e, string_hash_t event, float value) {
    NodeEventData data{event, e};
    data.payload.f32 = value;
    dispatch_broadcast(e, data);
}

inline void broadcast(entity_t e, string_hash_t event, int value) {
    NodeEventData data{event, e};
    data.payload.i32 = value;
    dispatch_broadcast(e, data);
}

inline void broadcast(entity_t e, string_hash_t event, const char* payload) {
    NodeEventData data{event, e, {payload}};
    dispatch_broadcast(e, data);
}

inline void notify_parents(entity_t e, string_hash_t event, const char* payload = "") {
    NodeEventData data{event, e, {payload}};
    dispatch_bubble(e, data);
}

}

ECX_COMP_TYPE_CXX(ek::NodeEventHandler);
