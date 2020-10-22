#pragma once

#include <ecxx/ecxx.hpp>
#include <ek/util/type_index.hpp>

namespace ek {

class script_cpp_base {
public:

    explicit script_cpp_base(uint32_t type_id)
            : type_id_{type_id} {

    }

    virtual ~script_cpp_base();

    void link_to_entity(ecs::entity e) {
        entity_ = e;
    }

    virtual void start() {}

    virtual void update(float dt) {}

    virtual void draw() {}

    virtual void gui_gizmo() {}

    virtual void gui_inspector() {}

    uint32_t get_type_id() {
        return type_id_;
    }

    template<typename Component>
    inline Component& get() {
        return ecs::get<Component>(entity_);
    }

    template<typename Component>
    inline Component& get_or_create() {
        return ecs::get_or_create<Component>(entity_);
    }

    [[nodiscard]] ecs::entity find_child(const std::string& name) const;

protected:
    ecs::entity entity_;
    uint32_t type_id_;
};

template<typename T>
class script_cpp : public script_cpp_base {
public:
    script_cpp()
            : script_cpp_base{type_index<T, script_cpp_base>::value} {

    }

    ~script_cpp() override = default;
};

struct script_holder {
    std::vector<std::unique_ptr<script_cpp_base>> list;
};

}

#define EK_DECL_SCRIPT_CPP(T) class T : public ::ek::script_cpp<T>