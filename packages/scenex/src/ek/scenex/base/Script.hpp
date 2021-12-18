#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/Type.hpp>
#include <ek/time.h>
#include <ek/util/StaticSignal.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/ds/Pointer.hpp>
#include "../base/TimeLayer.hpp"

namespace ek {

struct Updater {
    StaticSignal<ecs::EntityApi, TimeLayer> onUpdate{};
    TimeLayer timeLayer = TimeLayer::Root;
};

ECX_TYPE(5, Updater);

class ScriptBase {
public:

    explicit ScriptBase(int type_id) :
            type_id_{type_id} {

    }

    virtual ~ScriptBase();

    void link_to_entity(ecs::EntityApi e) {
        entity_ = e;
    }

    virtual void start() {}

    virtual void update(float) {}

    virtual void draw() {}

    virtual void gui_gizmo() {}

    virtual void gui_inspector() {}

    [[nodiscard]]
    uint32_t get_type_id() const {
        return type_id_;
    }

    template<typename Component>
    inline Component& get() {
        return entity_.get<Component>();
    }

    template<typename Component>
    inline Component& get_or_create() {
        return entity_.get_or_create<Component>();
    }

    [[nodiscard]] ecs::EntityApi find_child(const char* name) const;

protected:
    ecs::EntityApi entity_;
    int type_id_;
};

template<typename T>
class Script : public ScriptBase {
public:
    Script() : ScriptBase{TypeIndex<T, ScriptBase>::value} {
    }

    ~Script() override = default;
};

struct ScriptHolder {
    ecs::EntityApi owner;
    Array<Pointer<ScriptBase>> list;

    void link(ecs::EntityApi owner_);

    template<typename T>
    T& make() {
        auto& r = list.emplace_back(Pointer<ScriptBase>{Pointer<T>::make()});
        r->link_to_entity(owner);
        r->start();
        return static_cast<T&>(*r);
    }
};

ECX_TYPE(4, ScriptHolder);

class ScriptDrawable2D : public Drawable2D<ScriptDrawable2D> {
public:
    ecs::EntityApi entity_;
    Pointer<IDrawable2D> delegate;

    ScriptDrawable2D(ecs::EntityApi entity, Pointer<IDrawable2D> delegate_) :
            Drawable2D(),
            entity_{entity},
            delegate{std::move(delegate_)} {

    }

    void draw() override {
        if (delegate) {
            delegate->draw();
        }
        auto* holder = entity_.tryGet<ScriptHolder>();
        EK_ASSERT(holder);
        for (auto& script : holder->list) {
            script->draw();
        }
    }

    [[nodiscard]]
    Rect2f getBounds() const override { return delegate ? delegate->getBounds() : Rect2f{}; }

    [[nodiscard]]
    bool hitTest(Vec2f point) const override { return delegate && delegate->hitTest(point); }
};

template<typename S>
inline S& assignScript(ecs::EntityApi e) {
    auto& holder = e.get_or_create<ScriptHolder>();
    holder.link(e);
    return holder.make<S>();
}

template<typename S>
inline S& findScript(ecs::EntityApi e) {
    const auto interest_type_id = TypeIndex<S, ScriptBase>::value;
    auto& h = e.get<ScriptHolder>();
    for (auto& script : h.list) {
        if (script && script->get_type_id() == interest_type_id) {
            return static_cast<S&>(*script);
        }
    }
    abort();
}

void updateScripts();

EK_TYPE_INDEX_T(IDrawable2D, ScriptDrawable2D, 6);

}

#define EK_DECL_SCRIPT_CPP(Tp,...) class Tp : public ek::Script<Tp>

