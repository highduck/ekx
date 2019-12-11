#pragma once

#include <ek/graphics/blend_mode.hpp>
#include <ek/util/common_macro.hpp>
#include <ek/math/box.hpp>
#include <ek/math/mat4x4.hpp>
#include <functional>

namespace ek {

class program_t;

class texture_t;

class batch_state_manager : private disable_copy_assign_t {

    struct state_t {
        blend_mode blend = blend_mode::nop;
        const program_t* program = nullptr;
        const texture_t* texture = nullptr;

        bool operator==(const state_t& a) const {
            return (blend == a.blend && program == a.program && texture == a.texture);
        }

        bool operator!=(const state_t& a) const {
            return (blend != a.blend || program != a.program || texture != a.texture);
        }
    };

public:

    batch_state_manager() = default;

    inline state_t& prev() {
        return states_chain_[0];
    }

    inline state_t& current() {
        return states_chain_[1];
    }

    inline state_t& next() {
        return states_chain_[2];
    }

    void apply();

    void disable_scissors();

    void set_scissors(const rect_f& rc);

    void set_blend_mode(blend_mode blendMode);

    void set_texture(const texture_t* tex);

    void set_program(const program_t* program);

    void set_projection(const mat4f& mat);

    void set_transform(const mat4f& mat);

    void transform(const mat4f& mat) {
        set_transform(mat);
    }

    inline const mat4f& transform() const {
        return matrix_transform_;
    }

    void clear();

    void invalidate();

    inline bool is_changed() const {
        return changed_;
    }

    inline const mat4f& get_projection() const {
        return matrix_projection_;
    }

private:
    state_t states_chain_[3];

    rect_f scissors_{};
    bool scissors_enabled_ = false;
    bool scissors_dirty_ = false;

    mat4f matrix_projection_{};
    mat4f matrix_transform_{};
    mat4f matrix_combined_{};

    bool reset_matrix_ = false;
    bool dirty_matrix_ = false;
    bool changed_ = true;
};
}