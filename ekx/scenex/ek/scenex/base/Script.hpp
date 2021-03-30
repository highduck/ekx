#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/type_index.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <vector>

namespace ek {

class ScriptBase {
public:

    explicit ScriptBase(uint32_t type_id) :
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

    uint32_t get_type_id() {
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

    [[nodiscard]] ecs::EntityApi find_child(const std::string& name) const;

protected:
    ecs::EntityApi entity_;
    uint32_t type_id_;
};

template<typename T>
class Script : public ScriptBase {
public:
    Script() :
            ScriptBase{type_index<T, ScriptBase>::value} {

    }

    ~Script() override = default;
};

struct ScriptHolder {
    ecs::EntityApi owner;
    std::vector<std::unique_ptr<ScriptBase>> list;

    void link(ecs::EntityApi owner_);

    template<typename T>
    T& make() {
        auto& r = *list.emplace_back(std::make_unique<T>());
        r.link_to_entity(owner);
        r.start();
        return static_cast<T&>(r);
    }
};

class ScriptDrawable2D : public Drawable2D<ScriptDrawable2D> {
public:
    ecs::EntityApi entity_;
    std::unique_ptr<IDrawable2D> delegate;

    ScriptDrawable2D(ecs::EntityApi entity, std::unique_ptr<IDrawable2D> delegate_) :
            Drawable2D(),
            entity_{entity},
            delegate{std::move(delegate_)} {

    }

    void draw() override {
        if (delegate) {
            delegate->draw();
        }
        auto* holder = entity_.tryGet<ScriptHolder>();
        assert(holder);
        for (auto& script : holder->list) {
            script->draw();
        }
    }

    [[nodiscard]]
    rect_f getBounds() const override { return delegate ? delegate->getBounds() : rect_f{}; }

    [[nodiscard]]
    bool hitTest(float2 point) const override { return delegate && delegate->hitTest(point); }
};

template<typename S>
inline S& assignScript(ecs::EntityApi e) {
    auto& holder = e.get_or_create<ScriptHolder>();
    holder.link(e);
    return holder.make<S>();
}

template<typename S>
inline S& findScript(ecs::EntityApi e) {
    const auto interest_type_id = type_index<S, ScriptBase>::value;
    auto& h = e.get<ScriptHolder>();
    for (auto& script : h.list) {
        if (script && script->get_type_id() == interest_type_id) {
            return static_cast<S&>(*script);
        }
    }
    abort();
}

void updateScripts();

}

#define EK_DECL_SCRIPT_CPP(T) class T : public ::ek::Script<T>
