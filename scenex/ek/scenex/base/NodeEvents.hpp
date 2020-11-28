#pragma once

#include <unordered_map>
#include <ek/util/signals.hpp>
#include <ecxx/ecxx.hpp>
#include <string>
#include <any>
#include <utility>

namespace ek {

using NodeEventType = std::string;

struct NodeEventData {
    NodeEventType type;
    ecs::entity source;
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

}