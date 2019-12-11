#pragma once

#include <unordered_map>
#include <ek/util/signals.hpp>
#include <ecxx/ecxx.hpp>
#include <string>
#include <any>

namespace ek {

template<typename T>
class basic_event_handler {
public:

    template<typename S>
    using signal_type = signal_t<const S&>;

    signal_type<T>& require_signal_for_event_type(const std::string& type) {
        return map_[type];
    }

    void on_event(const std::string& type, std::function<void(const T&)> listener) {
        require_signal_for_event_type(type).add(listener);
    }

    void on(const std::string& type, std::function<void(const T&)> listener) {
        require_signal_for_event_type(type).add(listener);
    }

    void once(const std::string& type, std::function<void(const T&)> listener) {
        require_signal_for_event_type(type).add_once(listener);
    }

    void off(const std::string& type, std::function<void(const T&)> listener) {
        auto it = map_.find(type);
        if (it != map_.end()) {
            it->second.remove(listener);
        }
    }

    T emit(const T& event) {
        auto it = map_.find(event.type);
        if (it != map_.end()) {
            it->second.emit(event);
        }
        return event;
    }

private:
    std::unordered_map<std::string, signal_type<T>> map_;
};

struct event_data {
    std::string type;
    ecs::entity source;
    std::any payload;
    mutable bool processed = false;
};

using event_handler_t = basic_event_handler<event_data>;

}