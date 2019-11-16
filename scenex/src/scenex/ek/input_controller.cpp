#include "input_controller.h"

namespace scenex {

using namespace ek;

input_controller::input_controller(interactive_manager& interactions)
        : interactions_{interactions} {
    g_app.listen(this);
}

input_controller::~input_controller() = default;

void emulate_mouse_as_touch(const mouse_event_t& event, touch_state_t& data) {
    bool active_prev = data.active;
    if (!data.active && event.type == mouse_event_type::down) {
        data.active = true;
    } else if (data.active && (event.type == mouse_event_type::up || event.type == mouse_event_type::exit)) {
        data.active = false;
    }

    data.position.x = event.x;
    data.position.y = event.y;
    data.pressed = data.active;
    data.is_started_event = !active_prev && data.active;//e.type == "touchstart";
    data.is_ended_event = !data.active && active_prev;

    if (data.is_started_event) {
        data.start_position = data.position;
    }
    if (data.is_ended_event) {
        data.end_position = data.position;
    }
}

void input_controller::onMouseEvent(const mouse_event_t& event) {
    if (!hovered_by_editor_gui) {
        interactions_.handle_mouse_event(event);
    }

    //emulate_mouse_as_touch(event, get_or_create_touch(1u));
}

void update_touch(const touch_event_t& event, touch_state_t& data) {
    bool active_prev = data.active;
    data.active = event.type != touch_event_type::end;

    data.position.x = event.x;
    data.position.y = event.y;
    data.pressed = data.active;
    data.is_started_event = !active_prev && data.active;//e.type == "touchstart";
    data.is_ended_event = !data.active && active_prev;

    if (data.is_started_event) {
        data.start_position = data.position;
    }
    if (data.is_ended_event) {
        data.end_position = data.position;
    }
}

void input_controller::onTouchEvent(const touch_event_t& event) {
    if (!hovered_by_editor_gui) {
        interactions_.handle_touch_event(event);
    }

    update_touch(event, get_or_create_touch(event.id));
}

void input_controller::onKeyEvent(const key_event_t& event) {
    if (event.type == key_event_type::down) {
        if (event.code == key_code::Escape) {
            interactions_.send_back_button();
        }
    }

    if (event.code != key_code::Unknown) {
        auto& key = keys_[static_cast<size_t>(event.code)];
        if (event.type == key_event_type::down) {
            key.down = !key.state;
            key.state = true;//!keyboard_modifiers.ctrlKey;
            key.up = false;
        } else if (event.type == key_event_type::up) {
            key.down = false;
            key.state = false;
            key.up = true;
            //modifiers = none;
        }
    }
}

void input_controller::onAppEvent(const app_event_t& event) {
    if (event.type == app_event_type::back_button) {
        interactions_.send_back_button();
    } else if (event.type == app_event_type::paused) {
        interactions_.handle_system_pause();
    }
}

bool input_controller::is_key(key_code code) const {
    if (code != key_code::Unknown) {
        return keys_[static_cast<size_t>(code)].state;
    }
    return false;
}

bool input_controller::is_key_down(key_code code) const {
    if (code != key_code::Unknown) {
        return keys_[static_cast<size_t>(code)].down;
    }
    return false;
}

bool input_controller::is_key_up(key_code code) const {
    if (code != key_code::Unknown) {
        return keys_[static_cast<size_t>(code)].up;
    }
    return false;
}

void input_controller::on_frame_completed() {
    application_listener_t::on_frame_completed();

    // update keyboard state
    for (auto& key : keys_) {
        key.down = false;
        key.up = false;
        if (reset_keys_) {
            key.state = false;
            reset_keys_ = false;
        }
    }

    keyboard_text_.clear();

    // update touches info
    auto it = touches_.begin();
    while (it != touches_.end()) {
        if (!it->active) {
            it = touches_.erase(it);
        } else {
            ++it;
        }
    }
}

void input_controller::reset_keyboard() {
    reset_keys_ = true;
}

}