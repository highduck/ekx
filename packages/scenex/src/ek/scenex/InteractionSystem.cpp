#include "InteractionSystem.hpp"

#include <ecxx/ecxx.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/base/NodeEvents.hpp>
#include <ek/scenex/systems/hitTest.hpp>
#include <ek/scenex/2d/Camera2D.hpp>
#include <ek/scenex/2d/RenderSystem2D.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/base/Interactive.hpp>

// prev - 0
// curr - 1
entity_t* getTargets(uint32_t off) {
    return g_interaction_system.targetLists[(g_interaction_system.targetListIndex + off) & 1];
}

namespace ek {

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

bool interaction_system_list_contains_target(const entity_t list[32], entity_t e) {
    for(uint32_t i = 0; i < 32; ++i) {
        entity_t target = list[i];
        if(target == e) {
            return true;
        }
        if(!target) {
            break;
        }
    }
    return false;
}

void InteractionSystem::fireInteraction(string_hash_t event, bool prev, bool onlyIfChanged) {
    uint32_t off = prev ? 0 : 1;
    auto* targets = getTargets(off);
    auto* oppositeTargets = getTargets(off+1);

    for(uint32_t i = 0; i < 32; ++i) {
        entity_t target = targets[i];
        if (check_entity_alive(target)) {
            // TODO: we actually could check NodeEventHandler and dispatch events, if Interactive component set - we
            //  just update state
            auto* interactive = ecs::EntityApi{target}.tryGet<Interactive>();
            if (interactive && !(onlyIfChanged && interaction_system_list_contains_target(oppositeTargets, target))) {
                interactive->handle(target, event);
            }
        }
        else {
            break;
        }
    }
}

void InteractionSystem::handle_mouse_event(const ek_app_event* ev, vec2_t pos) {
    const ek_app_event_type type = ev->type;
    if (type == EK_APP_EVENT_MOUSE_DOWN) {
        mousePosition0_ = pos;
        pointerDown_ = true;
        fireInteraction(POINTER_EVENT_DOWN);
    } else if (type == EK_APP_EVENT_MOUSE_UP) {
        mousePosition0_ = pos;
        pointerDown_ = false;
        fireInteraction(POINTER_EVENT_UP);
    } else if (type == EK_APP_EVENT_MOUSE_MOVE) {
        mousePosition0_ = pos;
        mouseActive_ = true;
        update_interaction_system();
    } else if (type == EK_APP_EVENT_MOUSE_EXIT) {
        pointerDown_ = false;
        mouseActive_ = false;
        //update();
        fireInteraction(POINTER_EVENT_UP);
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
            update_interaction_system();
            fireInteraction(POINTER_EVENT_DOWN);
        }
    }

    if (touchID_ == touch_id) {
        if (type == EK_APP_EVENT_TOUCH_END) {
            touchID_ = 0;
            touchPosition0_ = {};
            pointerDown_ = false;
            fireInteraction(POINTER_EVENT_UP);
        } else {
            touchPosition0_ = pos;
        }
    }
}

void InteractionSystem::sendBackButton() {
    ecs::EntityApi r{root_};
    dispatch_interactive_event(r, {
            INTERACTIVE_EVENT_BACK_BUTTON,
            r.index,
            {"game"}
    });
}

void InteractionSystem::handle_system_pause() {
    ecs::EntityApi r{root_};
    broadcast(r, INTERACTIVE_EVENT_SYSTEM_PAUSE);
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
                auto target = hitTest2D(camera->root.index(), pointerWorldPosition);
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

ek_mouse_cursor InteractionSystem::searchInteractiveTargets(entity_t list[32]) {
    vec2_t pointer = {};
    ecs::EntityApi it;
    ecs::EntityRef camera;
    ecs::EntityRef drag_entity{dragEntity_};
    if (drag_entity.valid()) {
        it = drag_entity.ent();
        auto* interactive = it.tryGet<Interactive>();
        if (interactive && interactive->camera.valid()) {
            camera = interactive->camera;
            pointer = vec2_transform(pointerScreenPosition_, camera.get().get<Camera2D>().screenToWorldMatrix);
        }
    } else {
        it = globalHitTest(&pointer, &camera);
    }
    hitTarget_ = get_entity_passport(it.index);

    auto cursor = EK_MOUSE_CURSOR_PARENT;
    uint32_t len = 0;
    while (it) {
        auto* interactive = it.tryGet<Interactive>();
        if (interactive) {
            interactive->pointer = pointer;
            interactive->camera = ecs::EntityRef{camera};
            if (cursor == EK_MOUSE_CURSOR_PARENT) {
                cursor = interactive->cursor;
            }
            EK_ASSERT(len < 32);
            list[len++] = it.index;
            if (!interactive->bubble) {
                break;
            }
        }
        it = it.get<Node>().parent;
    }
    EK_ASSERT(len < 32);
    list[len] = 0;
    return cursor;
}

void InteractionSystem::drag(ecs::EntityApi entity) {
    dragEntity_ = get_entity_passport(entity.index);
}

ecs::EntityApi InteractionSystem::getHitTarget() const {
    return check_entity_passport(hitTarget_) ? ecs::EntityApi{hitTarget_} : nullptr;
}

}

ek::InteractionSystem g_interaction_system;

void init_interaction_system() {
   // g_interaction_system = new ek::InteractionSystem();
}

void update_interaction_system() {
    using namespace ek;

    auto* currTargets = getTargets(1);
    // clear current list
    currTargets[0] = 0;

    //pointer_global_space = float2::zero;
    auto cursor = EK_MOUSE_CURSOR_PARENT;
    bool changed = false;
    if (g_interaction_system.mouseActive_) {
        g_interaction_system.pointerScreenPosition_ = g_interaction_system.mousePosition0_;
        changed = true;
    } else if (g_interaction_system.touchID_ != 0) {
        g_interaction_system.pointerScreenPosition_ = g_interaction_system.touchPosition0_;
        changed = true;
    }

    if (changed) {
        cursor = g_interaction_system.searchInteractiveTargets(currTargets);
    }

    g_interaction_system.fireInteraction(POINTER_EVENT_OVER, false, true);
    g_interaction_system.fireInteraction(POINTER_EVENT_OUT, true, true);

    // swap target lists
    g_interaction_system.targetListIndex = (++g_interaction_system.targetListIndex) & 1;

    ek_app_set_mouse_cursor(cursor);
}

