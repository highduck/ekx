#pragma once

#include <platform/Application.h>
#include <scenex/interactive_manager.h>
#include <vector>

namespace scenex {

struct touch_state_t {
    uint64_t id = 0;
    float2 position;
    float2 start_position;
    bool is_started_event = false;
    float2 end_position;
    bool is_ended_event = false;
    bool pressed = false;
    bool active = false;

    touch_state_t() = default;

    explicit touch_state_t(uint64_t id_)
            : id{id_} {

    }
};

class input_controller : public ek::Application::Listener {
public:

    explicit input_controller(interactive_manager& interactions);

    ~input_controller() override;

    void onKeyEvent(const ek::KeyEvent& event) override;

    void onMouseEvent(const ek::MouseEvent& event) override;

    void onTouchEvent(const ek::TouchEvent& event) override;

    void onAppEvent(const ek::AppEvent& event) override;

    void onDrawFrame() override {};

    void on_frame_completed() override;

    bool is_key(ek::KeyEvent::Code code) const;

    bool is_key_down(ek::KeyEvent::Code code) const;

    bool is_key_up(ek::KeyEvent::Code code) const;

    void reset_keyboard();

    const std::vector<touch_state_t>& touches() const {
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
private:

    struct key_state_t {
        bool state = false;
        bool up = false;
        bool down = false;
    };

    interactive_manager& interactions_;
    std::string keyboard_text_;
    int keyboard_modifiers_;
    bool reset_keys_ = false;

    constexpr static size_t keys_count = static_cast<size_t>(ek::KeyEvent::Code::MaxCount);
    using keys_array = std::array<key_state_t, keys_count>;
    keys_array keys_;

    std::vector<touch_state_t> touches_;


//
//
//    public var down(default, null):FastStringMap<Bool> = new FastStringMap();
//    public var up(default, null):FastStringMap<Bool> = new FastStringMap();
//    public var state(default, null):FastStringMap<Bool> = new FastStringMap();
//
//    public function reset(resetState:Bool = true) {
//            for (key in down.keys())
//                down.set(key, false);
//
//            if (resetState) {
//                for (key in state.keys()) {
//                    state.set(key, false);
//                }
//            }
//
//            for (key in up.keys()) up.set(key, false);
//
//            text = null;
//        }
//    };

};

}


