#include "app.hpp"

#include <ek/ext/analytics/analytics.hpp>
#include <mutex>

namespace ek::app {

std::mutex event_queue_mtx;
std::vector<event_t> pool_queue{};
app_state g_app{};

void dispatch_init() {
    analytics::init(); // analytics before crash reporter on ios
}

void dispatch_device_ready() {
    g_app.on_device_ready();
}

void process_event(const event_t& event) {
    assert(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(event_type::max_count));

    g_app.on_event(event);
}

void dispatch_event(const event_t& event) {
    assert(static_cast<uint8_t>(event.type) < static_cast<uint8_t>(event_type::max_count));

    if (event.type == event_type::app_pause) {
        process_event(event);
    } else {
        event_queue_mtx.lock();
        assert(!g_app.event_queue_locked);
        g_app.event_queue_.emplace_back(event);
        event_queue_mtx.unlock();
    }
}

void dispatch_draw_frame() {
    event_queue_mtx.lock();
    // do not call dispatch_draw_frame recursively
    assert(!g_app.event_queue_locked);
    g_app.event_queue_locked = true;
    pool_queue = g_app.event_queue_;
    g_app.event_queue_.clear();
    g_app.event_queue_locked = false;
    event_queue_mtx.unlock();

    if (g_app.size_changed) {
        g_app.size_changed = false;
        process_event({event_type::app_resize});
    }

    for (const auto& event : pool_queue) {
        process_event(event);
    }

    g_app.on_frame_draw();
    g_app.on_frame_completed();
}

std::vector<std::string> arguments::to_vector() const {
    using namespace std;
    vector<string> result{};
    result.reserve(argc);
    for (int i = 0; i < argc; ++i) {
        result.emplace_back(argv[i]);
    }
    return result;
}

}