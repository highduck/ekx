#include "input_controller.hpp"

namespace ek {

input_controller::input_controller() = default;
input_controller::~input_controller() = default;

void input_controller::emulate_mouse_as_touch(const ek_app_event& event, touch_state_t& data) {
    bool active_prev = data.active;
    if (!data.active && event.type == EK_APP_EVENT_MOUSE_DOWN) {
        data.active = true;
    } else if (data.active && (event.type == EK_APP_EVENT_MOUSE_UP || event.type == EK_APP_EVENT_MOUSE_EXIT)) {
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

void input_controller::update_touch(const ek_app_event& event, touch_state_t& data) {
    bool active_prev = data.active;
    data.active = event.type != EK_APP_EVENT_TOUCH_END;

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

void input_controller::onEvent(const ek_app_event& event) {
    switch (event.type) {
        case EK_APP_EVENT_TOUCH_START:
        case EK_APP_EVENT_TOUCH_MOVE:
        case EK_APP_EVENT_TOUCH_END:
            if (!hovered_by_editor_gui) {
                g_interaction_system->handle_touch_event(event, screenCoordToGameDisplay({event.touch.x, event.touch.y}));
            }
            update_touch(event, get_or_create_touch(event.touch.id));
            break;
        case EK_APP_EVENT_MOUSE_DOWN:
        case EK_APP_EVENT_MOUSE_UP:
        case EK_APP_EVENT_MOUSE_MOVE:
        case EK_APP_EVENT_MOUSE_ENTER:
        case EK_APP_EVENT_MOUSE_EXIT:
            if (!hovered_by_editor_gui) {
                g_interaction_system->handle_mouse_event(event, screenCoordToGameDisplay({event.mouse.x, event.mouse.y}));
            }
            if (emulateTouch) {
                emulate_mouse_as_touch(event, get_or_create_touch(1u));
            }
            break;
        case EK_APP_EVENT_WHEEL:
            break;
        case EK_APP_EVENT_KEY_UP:
        case EK_APP_EVENT_KEY_DOWN:
            if (event.type == EK_APP_EVENT_KEY_DOWN && event.key.code == EK_KEYCODE_ESCAPE) {
                g_interaction_system->sendBackButton();
            }
            if (event.key.code != EK_KEYCODE_UNKNOWN) {
                auto& key = keys_[static_cast<size_t>(event.key.code)];
                if (event.type == EK_APP_EVENT_KEY_DOWN) {
                    key.down = !key.state;
                    key.state = true;//!keyboard_modifiers.ctrlKey;
                    key.up = false;
                } else if (event.type == EK_APP_EVENT_KEY_UP) {
                    key.down = false;
                    key.state = false;
                    key.up = true;
                    //modifiers = none;
                }
            }
            break;
        case EK_APP_EVENT_BACK_BUTTON:
            g_interaction_system->sendBackButton();
            break;
        case EK_APP_EVENT_PAUSE:
            g_interaction_system->handle_system_pause();
            break;
        default:
            break;
    }
}

bool input_controller::is_key(ek_key_code code) const {
    if (code != EK_KEYCODE_UNKNOWN && code < _EK_KEYCODE_NUM) {
        return keys_[code].state;
    }
    return false;
}

bool input_controller::is_key_down(ek_key_code code) const {
    if (code != EK_KEYCODE_UNKNOWN && code < _EK_KEYCODE_NUM) {
        return keys_[code].down;
    }
    return false;
}

bool input_controller::is_key_up(ek_key_code code) const {
    if (code != EK_KEYCODE_UNKNOWN && code < _EK_KEYCODE_NUM) {
        return keys_[code].up;
    }
    return false;
}

void input_controller::onPostFrame() {
    // update keyboard state
    for (auto& key : keys_) {
        key.down = false;
        key.up = false;
        if (reset_keys_) {
            key.state = false;
            reset_keys_ = false;
        }
    }

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

vec2_t input_controller::screenCoordToGameDisplay(const vec2_t screenPos) const {
    const auto size = display_->info.destinationViewport.size;
    const auto offset = display_->info.destinationViewport.position;
    const auto displaySize = display_->info.size;
    const float invScale = 1.0f / fmin(size.x / displaySize.x, size.y / displaySize.y);
    return invScale * (screenPos - offset);
}

}

ek::input_controller* g_input_controller = nullptr;
void init_input_controller(void) {
    EK_ASSERT(!g_input_controller);
    g_input_controller = new ek::input_controller();
}
