#include "InteractionSystem.hpp"

#include <ek/math/vec.hpp>
#include <ek/scenex/components/interactive.hpp>
#include <ek/scenex/utility/scene_management.hpp>
#include <ek/scenex/scene_system.hpp>
#include <ek/scenex/2d/Camera2D.hpp>

using namespace ek::app;

using namespace std;

namespace ek {

InteractionSystem::InteractionSystem(ecs::entity root) :
        root_{root} {

}

template<typename T>
inline bool contains(const vector<T>& vec, const T& value) {
    return find(vec.cbegin(), vec.cend(), value) != vec.cend();
}

bool dispatch_interactive_event(ecs::entity e, const event_data& data) {
    if (isTouchable(e)) {
        if (ecs::has<event_handler_t>(e)) {
            auto& eh = ecs::get<event_handler_t>(e);
            eh.emit(data);
            if (data.processed) {
                return true;
            }
        }
        auto it = ecs::get<Node>(e).child_last;
        while (it) {
            auto prev = ecs::get<Node>(it).sibling_prev;
            if (dispatch_interactive_event(it, data)) {
                return true;
            }
            it = prev;
        }
    }
    return false;
}

void InteractionSystem::process() {
    auto& prevTargets = getPrevTargets();
    auto& currTargets = getCurrentTargets();

    currTargets.clear();

    //pointer_global_space = float2::zero;
    auto cursor = interactive_t::mouse_cursor::parent;
    bool changed = false;
    if (mouseActive_) {
        pointerScreenPosition_ = mousePosition0_;
        changed = true;
    } else if (touchID_ != 0) {
        pointerScreenPosition_ = touchPosition0_;
        changed = true;
    }

    if (changed) {
        auto& cameras = Camera2D::getCameraQueue();
        float2 pointerWorldPosition{};
        for (int i = static_cast<int>(cameras.size()) - 1; i >= 0; --i) {
            auto* camera = cameras[i].tryGet<Camera2D>();
            if (camera && camera->enabled && camera->interactive &&
                camera->screenRect.contains(pointerScreenPosition_)) {
                pointerWorldPosition = camera->matrix.transform(pointerScreenPosition_);
                cursor = searchInteractiveTargets(pointerWorldPosition, camera->root, currTargets);
            }
        }

//        pointer = global_to_local(entity_, pointerScreenPosition_);
//        cursor = search_interactive_targets(entity_, targets_);
    }

    fireInteraction(InteractionEvent::PointerOver, false, true);
    fireInteraction(InteractionEvent::PointerOut, true, true);

    swapTargetLists();

    g_app.updateMouseCursor(cursor);
}

void InteractionSystem::fireInteraction(InteractionEvent event, bool prev, bool onlyIfChanged) {
    auto& targets = prev ? getPrevTargets() : getCurrentTargets();
    auto& oppositeTargets = prev ? getCurrentTargets() : getPrevTargets();

    for (auto target : targets) {
        if (target.valid()) {
            auto* interactive = target.tryGet<interactive_t>();
            if (interactive && !(onlyIfChanged && contains(oppositeTargets, target))) {
                interactive->handle(event);
            }
        }
    }
}

void InteractionSystem::handle_mouse_event(const event_t& ev) {

    if (ev.type == event_type::mouse_down) {
        mousePosition0_ = float2{ev.pos};
        pointerDown_ = true;
        fireInteraction(InteractionEvent::PointerDown);
    } else if (ev.type == event_type::mouse_up) {
        mousePosition0_ = float2{ev.pos};
        pointerDown_ = false;
        fireInteraction(InteractionEvent::PointerUp);
    } else if (ev.type == event_type::mouse_move) {
        mousePosition0_ = float2{ev.pos};
        mouseActive_ = true;
        process();
    } else if (ev.type == event_type::mouse_exit) {
        pointerDown_ = false;
        mouseActive_ = false;
        //update();
        fireInteraction(InteractionEvent::PointerUp);
    }
}

void InteractionSystem::handle_touch_event(const event_t& ev) {
    if (ev.type == event_type::touch_begin) {
        if (touchID_ == 0) {
            touchID_ = ev.id;
            touchPosition0_ = float2{ev.pos};
            mouseActive_ = false;
            pointerDown_ = true;
            process();
            fireInteraction(InteractionEvent::PointerDown);
        }
    }

    if (touchID_ == ev.id) {
        if (ev.type == event_type::touch_end) {
            touchID_ = 0;
            touchPosition0_ = float2::zero;
            pointerDown_ = false;
            fireInteraction(InteractionEvent::PointerUp);
        } else {
            touchPosition0_ = float2{ev.pos};
        }
    }
}

void InteractionSystem::sendBackButton() {
    dispatch_interactive_event(root_, {
            interactive_event::back_button,
            root_,
            "game"
    });
}

void InteractionSystem::handle_system_pause() {
    broadcast(root_, interactive_event::system_pause);
}

mouse_cursor InteractionSystem::searchInteractiveTargets(float2 pointer, ecs::entity node,
                                                           vector<ecs::entity>& list) {
    auto it = dragEntity_;
    if (!it.valid()) {
        it = hitTest(node, pointer);
    }
    hitTarget_ = it;

    auto cursor = mouse_cursor::parent;
    while (it) {
        auto* interactive = it.tryGet<interactive_t>();
        if (interactive) {
            if (cursor == mouse_cursor::parent) {
                cursor = interactive->cursor;
            }
            list.push_back(it);
            if (!interactive->bubble) {
                break;
            }
        }
        it = it.get<Node>().parent;
    }

    return cursor;
}

}
