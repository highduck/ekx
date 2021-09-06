#include "input_controller.hpp"

namespace ek {

using namespace ek::app;

static input_controller* gInputController = nullptr;

void gInputController_onEvent(const Event& e) {
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

void input_controller::emulate_mouse_as_touch(const Event& event, touch_state_t& data) {
    bool active_prev = data.active;
    if (!data.active && event.type == Event::MouseDown) {
        data.active = true;
    } else if (data.active && (event.type == Event::MouseUp || event.type == Event::MouseExit)) {
        data.active = false;
    }

    data.position = screenCoordToGameDisplay({event.mouse.x, event.mouse.y});
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

void input_controller::update_touch(const Event& event, touch_state_t& data) {
    bool active_prev = data.active;
    data.active = event.type != Event::TouchEnd;

    data.position = screenCoordToGameDisplay({event.touch.x, event.touch.y});
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

void input_controller::on_event(const Event& event) {
    switch (event.type) {
        case Event::TouchBegin:
        case Event::TouchMove:
        case Event::TouchEnd:
            if (!hovered_by_editor_gui) {
                interactions_.handle_touch_event(event, screenCoordToGameDisplay({event.touch.x, event.touch.y}));
            }
            update_touch(event, get_or_create_touch(event.touch.id));
            break;
        case Event::MouseDown:
        case Event::MouseUp:
        case Event::MouseMove:
        case Event::MouseScroll:
        case Event::MouseEnter:
        case Event::MouseExit:
            if (!hovered_by_editor_gui) {
                interactions_.handle_mouse_event(event, screenCoordToGameDisplay({event.mouse.x, event.mouse.y}));
            }
            if (emulateTouch) {
                emulate_mouse_as_touch(event, get_or_create_touch(1u));
            }
            break;
        case Event::KeyUp:
        case Event::KeyDown:
            if (event.type == Event::KeyDown && event.key.code == KeyCode::Escape) {
                interactions_.sendBackButton();
            }
            if (event.key.code != KeyCode::Unknown) {
                auto& key = keys_[static_cast<size_t>(event.key.code)];
                if (event.type == Event::KeyDown) {
                    key.down = !key.state;
                    key.state = true;//!keyboard_modifiers.ctrlKey;
                    key.up = false;
                } else if (event.type == Event::KeyUp) {
                    key.down = false;
                    key.state = false;
                    key.up = true;
                    //modifiers = none;
                }
            }
            break;
        case Event::BackButton:
            interactions_.sendBackButton();
            break;
        case Event::Pause:
            interactions_.handle_system_pause();
            break;
        default:
            break;
    }
}

bool input_controller::is_key(KeyCode code) const {
    if (code != KeyCode::Unknown) {
        return keys_[static_cast<size_t>(code)].state;
    }
    return false;
}

bool input_controller::is_key_down(KeyCode code) const {
    if (code != KeyCode::Unknown) {
        return keys_[static_cast<size_t>(code)].down;
    }
    return false;
}

bool input_controller::is_key_up(KeyCode code) const {
    if (code != KeyCode::Unknown) {
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