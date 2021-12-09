#pragma once

#include <ek/app/app.hpp>
#include <ek/scenex/InteractionSystem.hpp>
#include <ek/ds/Array.hpp>
#include <ek/ds/String.hpp>
#include "GameAppListener.hpp"
#include "GameDisplay.hpp"

namespace ek {

struct touch_state_t {
    uint64_t id = 0;
    Vec2f position;
    Vec2f start_position;
    bool is_started_event = false;
    Vec2f end_position;
    bool is_ended_event = false;
    bool pressed = false;
    bool active = false;

    touch_state_t() = default;

    explicit touch_state_t(uint64_t id_) : id{id_} {

    }
};

class input_controller final {
public:

    input_controller(InteractionSystem& interactions, GameDisplay& display);

    ~input_controller();

    void onEvent(const app::Event& event);

    void onPostFrame();

    [[nodiscard]]
    bool is_key(app::KeyCode code) const;

    [[nodiscard]]
    bool is_key_down(app::KeyCode code) const;

    [[nodiscard]]
    bool is_key_up(app::KeyCode code) const;

    void reset_keyboard();

    [[nodiscard]]
    const Array<touch_state_t>& touches() const {
        return touches_;
    }

    touch_state_t& get_or_create_touch(uint64_t id) {
        for (auto& touch : touches_) {
            if (touch.id == id) {
                return touch;
            }
        }
        return touches_.emplace_back(id);
    }

    bool hovered_by_editor_gui = false;
    bool emulateTouch = false;
private:
    [[nodiscard]] Vec2f screenCoordToGameDisplay(Vec2f pos) const;

    void emulate_mouse_as_touch(const app::Event& event, touch_state_t& data);

    void update_touch(const app::Event& event, touch_state_t& data);

    struct key_state_t {
        bool state = false;
        bool up = false;
        bool down = false;
    };

    GameDisplay& display_;
    InteractionSystem& interactions_;
    //int keyboard_modifiers_{};
    bool reset_keys_ = false;

    constexpr static size_t keys_count = static_cast<size_t>(app::KeyCode::MaxCount);
    key_state_t keys_[keys_count];

    Array<touch_state_t> touches_;
};

EK_DECLARE_TYPE(input_controller);

}


