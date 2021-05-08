#include "InteractionSystem.hpp"

#include <ecxx/ecxx.hpp>
#include <ek/math/vec.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/systems/hitTest.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/RenderSystem2D.hpp>
#include <ek/scenex/base/Node.hpp>
//#include <ek/scenex/SceneFactory.hpp>

using namespace ek::app;

namespace ek {

InteractionSystem::InteractionSystem(ecs::EntityApi root) :
        root_{root} {

}

template<typename T>
inline bool contains(const Array<T>& vec, const T& value) {
    for (auto e : vec) {
        if (e == value) {
            return true;
        }
    }
    return false;
}

bool dispatch_interactive_event(ecs::EntityApi e, const NodeEventData& data) {
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
        cursor = searchInteractiveTargets(currTargets);
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

void InteractionSystem::handle_mouse_event(const event_t& ev, float2 pos) {

    if (ev.type == event_type::mouse_down) {
        mousePosition0_ = pos;
        pointerDown_ = true;
        fireInteraction(InteractionEvent::PointerDown);
    } else if (ev.type == event_type::mouse_up) {
        mousePosition0_ = pos;
        pointerDown_ = false;
        fireInteraction(InteractionEvent::PointerUp);
    } else if (ev.type == event_type::mouse_move) {
        mousePosition0_ = pos;
        mouseActive_ = true;
        process();
    } else if (ev.type == event_type::mouse_exit) {
        pointerDown_ = false;
        mouseActive_ = false;
        //update();
        fireInteraction(InteractionEvent::PointerUp);
    }
}

void InteractionSystem::handle_touch_event(const event_t& ev, float2 pos) {
    if (ev.type == event_type::touch_begin) {
        if (touchID_ == 0) {
            touchID_ = ev.id;
            touchPosition0_ = pos;
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
            touchPosition0_ = pos;
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

ecs::EntityApi InteractionSystem::globalHitTest(float2& worldSpacePointer, ecs::EntityRef& capturedCamera) {
    auto& cameras = Camera2D::getCameraQueue();
    for (int i = static_cast<int>(cameras.size()) - 1; i >= 0; --i) {
        auto e = cameras[i];
        if (e.valid()) {
            auto* camera = e.get().tryGet<Camera2D>();
            if (camera && camera->enabled && camera->interactive &&
                camera->screenRect.contains(pointerScreenPosition_)) {
                const auto pointerWorldPosition = camera->screenToWorldMatrix.transform(pointerScreenPosition_);
                auto target = hitTest2D(ecs::the_world, camera->root.index(), pointerWorldPosition);
                if (target != 0) {
                    worldSpacePointer = pointerWorldPosition;
                    capturedCamera = e;
                    return ecs::EntityApi{target};
                }
            }
        }
    }
    return nullptr;
}

mouse_cursor InteractionSystem::searchInteractiveTargets(Array<ecs::EntityApi>& list) {
    float2 pointer{};
    ecs::EntityApi it;
    ecs::EntityRef camera;
    if (dragEntity_.valid()) {
        it = dragEntity_.ent();
        auto* interactive = it.tryGet<Interactive>();
        if (interactive && interactive->camera.valid()) {
            camera = interactive->camera;
            pointer = camera.get().get<Camera2D>().screenToWorldMatrix.transform(pointerScreenPosition_);
        }
    } else {
        it = globalHitTest(pointer, camera);
    }
    hitTarget_ = ecs::EntityRef{it};

    auto cursor = mouse_cursor::parent;
    while (it) {
        auto* interactive = it.tryGet<Interactive>();
        if (interactive) {
            interactive->pointer = pointer;
            interactive->camera = ecs::EntityRef{camera};
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

void InteractionSystem::drag(ecs::EntityApi entity) {
    dragEntity_ = ecs::EntityRef{entity};
}

ecs::EntityApi InteractionSystem::getHitTarget() const {
    return hitTarget_.valid() ? hitTarget_.ent() : nullptr;
}

}
