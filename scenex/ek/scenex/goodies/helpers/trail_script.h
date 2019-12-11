#pragma once

#include <ek/math/vec.hpp>
#include <ek/scenex/components/script_t.h>
#include <ek/scenex/config/ecs.h>

namespace ek {

struct trail_node {
    float2 position;
    float energy = 1.0f;
    float alpha = 1.0f;

    trail_node() noexcept = default;

    trail_node(float2 pos, float energy_, float alpha_) noexcept
            : position{pos},
              energy{energy_},
              alpha{alpha_} {

    }
};

class trail_script : public script {
public:

    float2 offset;
    float drain_speed = 1.0f;
    float width = 20.0f;
    float segment_distance_max = 10.0f;
    float min_width = 5.0f;
    float particles_per_second = 15.0f;

    void draw() override;

    void update(float dt) override;

    void track(ecs::entity e);

private:

    void update_trail(float dt);

    void update_position(const float2& position_next, float dt);

    void create_node(const float2& pos);

    void update_mesh();

    ecs::entity tracked_target_;
    float2 position_;
    float2 position_last_;
    std::vector<trail_node> nodes_;
    float particles_gen_ = 0.0f;
    std::vector<float2> vertex_list_;
    bool auto_update_ = true;
};

}


