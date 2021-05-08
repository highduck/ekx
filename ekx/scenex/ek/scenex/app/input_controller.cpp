#include "input_controller.hpp"

namespace ek {

using namespace ek::app;

static input_controller* gInputController = nullptr;

void gInputController_onEvent(const event_t& e) {
    EK_ASSERT(gInputController != nullptr);
    gInputController->on_event(e);
}

void gInputController_onFrameCompleted() {
    EK_ASSERT(gInputController != nullptr);
    gInputController->on_frame_completed();
}

input_controller::input_controller(InteractionSystem& interactions, GameDisplay& display) :
        interactions_{interactions},
        display_{display} {

    gInputController = this;
    g_app.on_event += gInputController_onEvent;
    g_app.on_frame_completed += gInputController_onFrameCompleted;
}

input_controller::~input_controller() {
    gInputController = nullptr;
    g_app.on_event -= gInputController_onEvent;
    g_app.on_frame_completed -= gInputController_onFrameCompleted;
}

void input_controller::emulate_mouse_as_touch(const event_t& event, touch_state_t& data) {
    bool active_prev = data.active;
    if (!data.active && event.type == event_type::mouse_down) {
        data.active = true;
    } else if (data.active && (event.type == event_type::mouse_up || event.type == event_type::mouse_exit)) {
        data.active = false;
    }

    data.position = screenCoordToGameDisplay(event.pos);
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

void input_controller::update_touch(const event_t& event, touch_state_t& data) {
    bool active_prev = data.active;
    data.active = event.type != event_type::touch_end;

    data.position = screenCoordToGameDisplay(event.pos);
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

void input_controller::on_event(const event_t& event) {
    switch (event.type) {
        case event_type::touch_begin:
        case event_type::touch_move:
        case event_type::touch_end:
            if (!hovered_by_editor_gui) {
                interactions_.handle_touch_event(event, screenCoordToGameDisplay(event.pos));
            }
            update_touch(event, get_or_create_touch(event.id));
            break;
        case event_type::mouse_down:
        case event_type::mouse_up:
        case event_type::mouse_move:
        case event_type::mouse_scroll:
        case event_type::mouse_enter:
        case event_type::mouse_exit:
            if (!hovered_by_editor_gui) {
                interactions_.handle_mouse_event(event, screenCoordToGameDisplay(event.pos));
            }
            if (emulateTouch) {
                emulate_mouse_as_touch(event, get_or_create_touch(1u));
            }
            break;
        case event_type::key_up:
        case event_type::key_down:
            if (event.type == event_type::key_down && event.code == key_code::Escape) {
                interactions_.sendBackButton();
            }
            if (event.code != key_code::unknown) {
                auto& key = keys_[static_cast<size_t>(event.code)];
                if (event.type == event_type::key_down) {
                    key.down = !key.state;
                    key.state = true;//!keyboard_modifiers.ctrlKey;
                    key.up = false;
                } else if (event.type == event_type::key_up) {
                    key.down = false;
                    key.state = false;
                    key.up = true;
                    //modifiers = none;
                }
            }
            break;
        case event_type::app_back_button:
            interactions_.sendBackButton();
            break;
        case event_type::app_pause:
            interactions_.handle_system_pause();
            break;
        default:
            break;
    }
}

bool input_controller::is_key(key_code code) const {
    if (code != key_code::unknown) {
        return keys_[static_cast<size_t>(code)].state;
    }
    return false;
}

bool input_controller::is_key_down(key_code code) const {
    if (code != key_code::unknown) {
        return keys_[static_cast<size_t>(code)].down;
    }
    return false;
}

bool input_controller::is_key_up(key_code code) const {
    if (code != key_code::unknown) {
        return keys_[static_cast<size_t>(code)].up;
    }
    return false;
}

void input_controller::on_frame_completed() {
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
    unsigned i = 0;
    while (i < touches_.size()) {
        if (touches_[i].active) {
            ++i;
        } else {
            touches_.eraseAt(i);
        }
    }
}

void input_controller::reset_keyboard() {
    reset_keys_ = true;
}

float2 input_controller::screenCoordToGameDisplay(float2 screenPos) const {
    const auto size = display_.info.destinationViewport.size;
    const auto offset = display_.info.destinationViewport.position;
    const auto displaySize = display_.info.size;
    const float invScale = 1.0f / fmin(size.x / displaySize.x, size.y / displaySize.y);
    return invScale * (screenPos - offset);
}

}