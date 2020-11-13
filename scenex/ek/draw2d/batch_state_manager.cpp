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
        mvpChanged = true;
    }

    if (is_texture_changed) {
        if (curr.texture && curr.program) {
            curr.texture->bind(curr.program->u_image0_unit);
        }
        prev.texture = curr.texture;
    }

    if (mvpChanged && curr.program) {
        curr.program->set_uniform(graphics::program_uniforms::mvp, mvp);
    }
    mvpChanged = false;

    if (prev.scissors != curr.scissors) {
        graphics::set_scissors(curr.scissors.x, curr.scissors.y, curr.scissors.width, curr.scissors.height);
        prev.scissors = curr.scissors;
    }
}

void batch_state_context::set_scissors(const rect_f& rc) {
    if (rc != curr.scissors) {
        anyChanged = true;
    }
    next.scissors = rc;
}

void batch_state_context::set_blend_mode(graphics::blend_mode blending) {
    if (curr.blend != blending) {
        anyChanged = true;
    }
    next.blend = blending;
}

void batch_state_context::set_texture(const graphics::texture_t* tex) {
    if (curr.texture != tex) {
        anyChanged = true;
    }
    next.texture = tex;
}

void batch_state_context::set_program(const graphics::program_t* program) {
    if (curr.program != program) {
        anyChanged = true;
    }
    next.program = program;
}

void batch_state_context::set_mvp(const mat4f& m) {
    mvp = m;
    mvpChanged = true;
    anyChanged = true;
}

void batch_state_context::clear() {
    anyChanged = true;
    prev = {};
    curr = {};
    next = {};
}

void batch_state_context::invalidate() {
    if (anyChanged) {
        curr = next;
        anyChanged = false;
    }
}

}