#include "input_controller.h"

namespace scenex {

using namespace ek;

input_controller::input_controller(interactive_manager& interactions)
        : interactions_{interactions} {
    gApp.listen(this);
}

input_controller::~input_controller() {
    // TODO:
//    gApp.unlisten(this);
}

void emulate_mouse_as_touch(const MouseEvent& event, touch_state_t& data) {
    bool active_prev = data.active;
    if (!data.active && event.type == MouseEvent::Type::Down) {
        data.active = true;
    } else if (data.active && (event.type == MouseEvent::Type::Up || event.type == MouseEvent::Type::Exit)) {
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

void input_controller::onMouseEvent(const MouseEvent& event) {
    if (!hovered_by_editor_gui) {
        interactions_.handle_mouse_event(event);
    }

    //emulate_mouse_as_touch(event, get_or_create_touch(1u));
}

void update_touch(const TouchEvent& event, touch_state_t& data) {
    bool active_prev = data.active;
    data.active = event.type != TouchEvent::Type::End;

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

void input_controller::onTouchEvent(const TouchEvent& event) {
    if (!hovered_by_editor_gui) {
        interactions_.handle_touch_event(event);
    }

    update_touch(event, get_or_create_touch(event.id));
}

void input_controller::onKeyEvent(const KeyEvent& event) {
    if (event.type == KeyEvent::Type::Down) {
        if (event.code == KeyEvent::Code::Escape) {
            interactions_.send_back_button();
        }
    }

    if (event.code != KeyEvent::Code::Unknown) {
        auto& key = keys_[static_cast<size_t>(event.code)];
        if (event.type == KeyEvent::Type::Down) {
            key.down = !key.state;
            key.state = true;//!keyboard_modifiers.ctrlKey;
            key.up = false;
        } else if (event.type == KeyEvent::Type::Up) {
            key.down = false;
            key.state = false;
            key.up = true;
            //modifiers = none;
        }
    }
}

void input_controller::onAppEvent(const AppEvent& event) {
    if (event.type == AppEvent::Type::BackButton) {
        interactions_.send_back_button();
    } else if (event.type == AppEvent::Type::Paused) {
        interactions_.handle_system_pause();
    }
}

bool input_controller::is_key(ek::KeyEvent::Code code) const {
    if (code != KeyEvent::Code::Unknown) {
        return keys_[static_cast<size_t>(code)].state;
    }
    return false;
}

bool input_controller::is_key_down(ek::KeyEvent::Code code) const {
    if (code != KeyEvent::Code::Unknown) {
        return keys_[static_cast<size_t>(code)].down;
    }
    return false;
}

bool input_controller::is_key_up(ek::KeyEvent::Code code) const {
    if (code != KeyEvent::Code::Unknown) {
        return keys_[static_cast<size_t>(code)].up;
    }
    return false;
}

void input_controller::on_frame_completed() {
    Listener::on_frame_completed();

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