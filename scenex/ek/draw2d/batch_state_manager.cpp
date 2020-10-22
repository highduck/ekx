#include "batch_state_manager.hpp"

#include <ek/graphics/program.hpp>
#include <ek/graphics/texture.hpp>

namespace ek {

void batch_state_context::apply() {
    auto bl = curr.blend;
    if (bl != prev.blend) {
        graphics::set_blend_mode(bl);
        prev.blend = bl;
    }

    const bool is_program_changed = prev.program != curr.program;
    bool is_texture_changed = prev.texture != curr.texture;

    if (is_program_changed) {
        curr.program->use();
        prev.program = curr.program;
        is_texture_changed = true;
        mvp_changed = true;
    }

    if (is_texture_changed) {
        if (curr.texture && curr.program) {
            curr.texture->bind(curr.program->u_image0_unit);
        }
        prev.texture = curr.texture;
    }

    if (mvp_changed && curr.program) {
        curr.program->set_uniform(graphics::program_uniforms::mvp, mvp);
    }
    mvp_changed = false;

    if (scissors_dirty) {
        if (scissors_enabled) {
            graphics::set_scissors(
                    (int) scissors.x,
                    (int) scissors.y,
                    (int) scissors.width,
                    (int) scissors.height
            );
        } else {
            graphics::set_scissors();
        }
        scissors_dirty = false;
    }
}

void batch_state_context::disable_scissors() {
    if (scissors_enabled) {
        changed = true;
        scissors_dirty = true;
        scissors_enabled = false;
    }
}

void batch_state_context::set_scissors(const rect_f& rc) {
    changed = true;
    scissors_dirty = true;
    scissors = rc;
    scissors_enabled = true;
}

void batch_state_context::set_blend_mode(graphics::blend_mode blending) {
    if (curr.blend != blending) {
        changed = true;
    }
    next.blend = blending;
}

void batch_state_context::set_texture(const graphics::texture_t* tex) {
    if (curr.texture != tex) {
        changed = true;
    }
    next.texture = tex;
}

void batch_state_context::set_program(const graphics::program_t* program) {
    if (curr.program != program) {
        changed = true;
    }
    next.program = program;
}

void batch_state_context::set_mvp(const mat4f& m) {
    mvp = m;
    mvp_changed = true;
    changed = true;
}

void batch_state_context::clear() {
    changed = true;
    prev = {};
    curr = {};
    next = {};
    scissors_enabled = false;
}

void batch_state_context::invalidate() {
    if (!changed) {
        return;
    }
    curr = next;
    changed = false;
}

}