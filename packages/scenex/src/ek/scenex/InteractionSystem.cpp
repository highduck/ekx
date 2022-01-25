#include "InteractionSystem.hpp"

#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/systems/hitTest.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/RenderSystem2D.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

InteractionSystem::InteractionSystem() {
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
    auto cursor = EK_MOUSE_CURSOR_PARENT;
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

    fireInteraction(PointerEvent::Over, false, true);
    fireInteraction(PointerEvent::Out, true, true);

    swapTargetLists();

    ek_app_set_mouse_cursor(cursor);
}

void InteractionSystem::fireInteraction(PointerEvent event, bool prev, bool onlyIfChanged) {
    auto& targets = prev ? getPrevTargets() : getCurrentTargets();
    auto& oppositeTargets = prev ? getCurrentTargets() : getPrevTargets();

    for (auto target : targets) {
        if (target.isAlive()) {
            auto* interactive = target.tryGet<Interactive>();
            if (interactive && !(onlyIfChanged && oppositeTargets.find(target) != nullptr)) {
                interactive->handle(event);
            }
        }
    }
}

void InteractionSystem::handle_mouse_event(const ek_app_event* ev, vec2_t pos) {
    const ek_app_event_type type = ev->type;
    if (type == EK_APP_EVENT_MOUSE_DOWN) {
        mousePosition0_ = pos;
        pointerDown_ = true;
        fireInteraction(PointerEvent::Down);
    } else if (type == EK_APP_EVENT_MOUSE_UP) {
        mousePosition0_ = pos;
        pointerDown_ = false;
        fireInteraction(PointerEvent::Up);
    } else if (type == EK_APP_EVENT_MOUSE_MOVE) {
        mousePosition0_ = pos;
        mouseActive_ = true;
        process();
    } else if (type == EK_APP_EVENT_MOUSE_EXIT) {
        pointerDown_ = false;
        mouseActive_ = false;
        //update();
        fireInteraction(PointerEvent::Up);
    }
}

void InteractionSystem::handle_touch_event(const ek_app_event* ev, vec2_t pos) {
    const ek_app_event_type type = ev->type;
    const uint64_t touch_id = ev->touch.id;
    if (type == EK_APP_EVENT_TOUCH_START) {
        if (touchID_ == 0) {
            touchID_ = touch_id;
            touchPosition0_ = pos;
            mouseActive_ = false;
            pointerDown_ = true;
            process();
            fireInteraction(PointerEvent::Down);
        }
    }

    if (touchID_ == touch_id) {
        if (type == EK_APP_EVENT_TOUCH_END) {
            touchID_ = 0;
            touchPosition0_ = {};
            pointerDown_ = false;
            fireInteraction(PointerEvent::Up);
        } else {
            touchPosition0_ = pos;
        }
    }
}

void InteractionSystem::sendBackButton() {
    dispatch_interactive_event(root_, {
            interactive_event::back_button,
            root_,
            {"game"}
    });
}

void InteractionSystem::handle_system_pause() {
    broadcast(root_, interactive_event::system_pause);
}

ecs::EntityApi InteractionSystem::globalHitTest(vec2_t* worldSpacePointer, ecs::EntityRef* capturedCamera) {
    auto& cameras = Camera2D::getCameraQueue();
    for (int i = static_cast<int>(cameras.size()) - 1; i >= 0; --i) {
        auto e = cameras[i];
        if (e.valid()) {
            auto* camera = e.get().tryGet<Camera2D>();
            if (camera && camera->enabled && camera->interactive &&
                    rect_contains(camera->screenRect, pointerScreenPosition_)) {
                const auto pointerWorldPosition = vec2_transform(pointerScreenPosition_, camera->screenToWorldMatrix);
                auto target = hitTest2D(ecs::the_world, camera->root.index(), pointerWorldPosition);
                if (target != 0) {
                    *worldSpacePointer = pointerWorldPosition;
                    *capturedCamera = e;
                    return ecs::EntityApi{target};
                }
            }
        }
    }
    return nullptr;
}

ek_mouse_cursor InteractionSystem::searchInteractiveTargets(Array<ecs::EntityApi>& list) {
    vec2_t pointer = {};
    ecs::EntityApi it;
    ecs::EntityRef camera;
    if (dragEntity_.valid()) {
        it = dragEntity_.ent();
        auto* interactive = it.tryGet<Interactive>();
        if (interactive && interactive->camera.valid()) {
            camera = interactive->camera;
            pointer = vec2_transform(pointerScreenPosition_, camera.get().get<Camera2D>().screenToWorldMatrix);
        }
    } else {
        it = globalHitTest(&pointer, &camera);
    }
    hitTarget_ = ecs::EntityRef{it};

    auto cursor = EK_MOUSE_CURSOR_PARENT;
    while (it) {
        auto* interactive = it.tryGet<Interactive>();
        if (interactive) {
            interactive->pointer = pointer;
            interactive->camera = ecs::EntityRef{camera};
            if (cursor == EK_MOUSE_CURSOR_PARENT) {
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

ek::InteractionSystem* g_interaction_system = nullptr;
void init_interaction_system() {
    EK_ASSERT(!g_interaction_system);
    g_interaction_system = new ek::InteractionSystem();
}
