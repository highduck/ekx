#include "InteractionSystem.hpp"

#include <ek/math/vec.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/systems/hitTest.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/RenderSystem2D.hpp>
#include <ek/scenex/base/Node.hpp>
//#include <ek/scenex/utility/scene_management.hpp>

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

bool dispatch_interactive_event(ecs::entity e, const NodeEventData& data) {
    if (isTouchable(e)) {
        auto* eh = e.tryGet<NodeEventHandler>();
        if (eh) {
            eh->emit(data);
            if (data.processed) {
                return true;
            }
        }
        auto it = e.get<Node>().child_last;
        while (it) {
            auto prev = it.get<Node>().sibling_prev;
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
    auto cursor = Interactive::mouse_cursor::parent;
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
                pointerWorldPosition = camera->screenToWorldMatrix.transform(pointerScreenPosition_);
                cursor = searchInteractiveTargets(pointerWorldPosition, camera->root.ent(), currTargets);
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
        if (target.isAlive()) {
            auto* interactive = target.tryGet<Interactive>();
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
    ecs::entity it = nullptr;
    if (dragEntity_.valid()) {
        it = dragEntity_.ent();
    }
    else {
        it = ecs::entity{hitTest2D(ecs::the_world, node.index, pointer)};
    }
    hitTarget_ = ecs::EntityRef{it};

    auto cursor = mouse_cursor::parent;
    while (it) {
        auto* interactive = it.tryGet<Interactive>();
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
