#include "interactive_manager.h"

#include <stack>
#include <ek/math/vec.hpp>
#include <scenex/components/interactive_t.h>
#include <scenex/utility/scene_management.h>
#include <scenex/scene_system.h>
#include <scenex/2d/font.hpp>

namespace scenex {

template<typename T>
inline bool contains(const std::vector<T>& vec, const T& value) {
    return std::find(vec.cbegin(), vec.cend(), value) != vec.cend();
}

bool dispatch_interactive_event(ecs::entity e, const event_data& data) {
    if (is_touchable(e)) {
        if (ecs::has<event_handler_t>(e)) {
            auto& eh = ecs::get<event_handler_t>(e);
            eh.emit(data);
            if (data.processed) {
                return true;
            }
        }
        auto it = ecs::get<node_t>(e).child_last;
        while (it) {
            auto prev = ecs::get<node_t>(it).sibling_prev;
            if (dispatch_interactive_event(it, data)) {
                return true;
            }
            it = prev;
        }
    }
    return false;
}

void interactive_manager::update() {
    targets_.clear();
    //pointer_global_space = float2::zero;
    auto cursor = cursor_t::Auto;
    bool changed = false;
    if (mouse_active_) {
        pointer_global_space = primary_mouse_;
        changed = true;
    } else if (primary_touch_id_ != 0) {
        pointer_global_space = primary_touch_;
        changed = true;
    }

    if (changed) {
        pointer = global_to_local(entity_, pointer_global_space);
        cursor = search_interactive_targets(entity_, targets_);
    }

    for (auto target : targets_) {
        if (ecs::valid(target)
            && ecs::has<interactive_t>(target)
            && !contains(last_targets_, target)) {

            ecs::get<interactive_t>(target).set_pointer_over();
        }
    }

    for (auto target : last_targets_) {
        if (ecs::valid(target)
            && ecs::has<interactive_t>(target)
            && !contains(targets_, target)) {

            ecs::get<interactive_t>(target).set_pointer_out();
        }
    }

    last_targets_ = targets_;

    ek::gWindow.setCursor(cursor);
}

void interactive_manager::handle_mouse_event(const interactive_manager::mouse_event_t& ev) {
    if (ev.type == mouse_event_t::Type::Down) {
        primary_mouse_ = {ev.x, ev.y};
        pointer_down = true;
        for (auto target : last_targets_) {
            if (ecs::valid(target)) {
                ecs::get<interactive_t>(target).set_pointer_down();
            }
        }
    } else if (ev.type == mouse_event_t::Type::Up) {
        primary_mouse_ = {ev.x, ev.y};
        pointer_down = false;
        for (auto target : last_targets_) {
            if (ecs::valid(target)) {
                ecs::get<interactive_t>(target).set_pointer_up();
            }
        }
    } else if (ev.type == mouse_event_t::Type::Move) {
        primary_mouse_ = {ev.x, ev.y};
        mouse_active_ = true;
        update();
    } else if (ev.type == mouse_event_t::Type::Exit) {
        mouse_active_ = false;
        pointer_down = false;
        update();
    }
}

void interactive_manager::handle_touch_event(const touch_event_t& ev) {
    if (ev.type == touch_event_t::Type::Begin) {
        if (primary_touch_id_ == 0) {
            primary_touch_id_ = ev.id;
            primary_touch_ = {ev.x, ev.y};
            mouse_active_ = false;
            pointer_down = true;
            update();
            for (auto target : last_targets_) {
                if (ecs::valid(target)) {
                    ecs::get<interactive_t>(target).set_pointer_down();
                }
            }
        }
    }

    if (primary_touch_id_ == ev.id) {
        if (ev.type == touch_event_t::Type::End) {
            primary_touch_id_ = 0;
            primary_touch_ = float2::zero;
            pointer_down = false;
            for (auto target : last_targets_) {
                if (ecs::valid(target)) {
                    ecs::get<interactive_t>(target).set_pointer_up();
                }
            }
        } else {
            primary_touch_ = {ev.x, ev.y};
        }
    }
}

void interactive_manager::send_back_button() {
    dispatch_interactive_event(entity_, {"backButton", entity_, "game"});
}

void interactive_manager::handle_system_pause() {
    broadcast(entity_, "system_pause");
}

void interactive_manager::set_debug_hit(ecs::entity hit) {
    if (debug_hit_target_ != hit) {
        if (debug_hit_target_) {
            // TODO:
//            debug_hit_target_->events.off(PreDradrawBorders);
//            debug_hit_target_->events.off(OnDradrawBounds);
        }
        debug_hit_target_ = hit;
        if (debug_hit_target_) {
//            debug_hit_target_->events.on(PreDradrawBorders);
//            debug_hit_target_->events.on(OnDradrawBounds);
        }
    }
}

interactive_manager::cursor_t interactive_manager::search_interactive_targets(
        ecs::entity node,
        std::vector<ecs::entity>& out_entities) {

    auto target = hit_test(node, pointer);
    if (ecs::valid(drag_entity_)) {
        target = drag_entity_;
    }

    auto cursor = cursor_t::Auto;

    if (debug_hit_enabled) {
        set_debug_hit(target);
    }

    while (target) {
        if (ecs::has<interactive_t>(target)) {
            auto& data = ecs::get<interactive_t>(target);
            if (cursor == cursor_t::Auto) {
                cursor = data.cursor;
            }
            out_entities.push_back(target);
            if (!data.bubble) {
                break;
            }
        }
        target = ecs::get<node_t>(target).parent;
    }
    return cursor;
}

}
