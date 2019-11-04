#include "batch_state_manager.hpp"

#include <ek/locator.hpp>
#include <graphics/program.hpp>
#include <graphics/texture.hpp>

namespace ek {

void batch_state_manager::apply() {
    auto& graphics = resolve<graphics_t>();
    auto bl = current().blend;
    if (bl != prev().blend) {
        graphics.set_blend_mode(bl);
        prev().blend = bl;
    }

    const bool is_program_changed = prev().program != current().program;
    bool is_texture_changed = prev().texture != current().texture;

    if (is_program_changed) {
        current().program->use();
        prev().program = current().program;
        is_texture_changed = true;
        reset_matrix_ = true;
    }

    if (is_texture_changed) {
        if (current().texture && current().program) {
            current().texture->bind(current().program->u_image0_unit);
        }
        prev().texture = current().texture;
    }

    if (reset_matrix_) {
        current().program->set_uniform(program_uniforms::mvp, matrix_combined_);
        reset_matrix_ = false;
    }

    if (scissors_dirty_) {
        if (scissors_enabled_) {
            graphics.set_scissors(
                    (int) scissors_.x,
                    (int) scissors_.y,
                    (int) scissors_.width,
                    (int) scissors_.height
            );
        } else {
            graphics.set_scissors();
        }
        scissors_dirty_ = false;
    }
}

void batch_state_manager::disable_scissors() {
    if (scissors_enabled_) {
        changed_ = true;
        scissors_dirty_ = true;
        scissors_enabled_ = false;
    }
}

void batch_state_manager::set_scissors(const rect_f& rc) {
    changed_ = true;
    scissors_dirty_ = true;
    scissors_ = rc;
    scissors_enabled_ = true;
}

void batch_state_manager::set_blend_mode(blend_mode blendMode) {
    if (current().blend != blendMode) {
        changed_ = true;
    }
    next().blend = blendMode;
}

void batch_state_manager::set_texture(const texture_t* tex) {
    if (current().texture != tex) {
        changed_ = true;
    }
    next().texture = tex;
}

void batch_state_manager::set_program(const program_t* program) {
    if (current().program != program) {
        changed_ = true;
    }
    next().program = program;
}

void batch_state_manager::set_projection(const mat4f& mat) {
    matrix_projection_ = mat;
    dirty_matrix_ = true;
    changed_ = true;
}

void batch_state_manager::set_transform(const mat4f& mat) {
    matrix_transform_ = mat;
    dirty_matrix_ = true;
    changed_ = true;
}

void batch_state_manager::clear() {
    changed_ = true;
    states_chain_[0] = {};
    states_chain_[1] = {};
    states_chain_[2] = {};
    scissors_enabled_ = false;
}

void batch_state_manager::invalidate() {
    if (!changed_) {
        return;
    }
    states_chain_[1] = states_chain_[2];
    if (dirty_matrix_) {
        matrix_combined_ = matrix_projection_ * matrix_transform_;
        dirty_matrix_ = false;
        reset_matrix_ = true;
    }
    changed_ = false;
}

}