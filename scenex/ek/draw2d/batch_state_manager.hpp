#pragma once

#include <ek/graphics/graphics.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat4x4.hpp>
#include <functional>

namespace ek {

struct batch_state_context : private disable_copy_assign_t {

    struct state_t {
        graphics::blend_mode blend = graphics::blend_mode::nop;
        const graphics::program_t* program = nullptr;
        const graphics::texture_t* texture = nullptr;
        rect_f scissors{};

        bool operator==(const state_t& a) const {
            return (blend == a.blend && program == a.program && texture == a.texture);
        }

        bool operator!=(const state_t& a) const {
            return (blend != a.blend || program != a.program || texture != a.texture);
        }
    };

    state_t prev;
    state_t curr;
    state_t next;

    mat4f mvp{};

    bool mvpChanged = false;
    bool anyChanged = true;

    void apply();

    void set_scissors(const rect_f& rc);

    void set_blend_mode(graphics::blend_mode blending);

    void set_texture(const graphics::texture_t* tex);

    void set_program(const graphics::program_t* program);

    void set_mvp(const mat4f& m);

    void clear();

    void invalidate();
};

}