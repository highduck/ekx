#pragma once

#include <ek/math/vec.hpp>
#include <ek/scenex/components/script.hpp>
#include <ecxx/ecxx.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/game_time.hpp>

namespace ek {

struct Trail2DNode;

struct Trail2D {
    struct Node {
        float2 position{};
        float energy = 1.0f;
        float alpha = 1.0f;
        float scale = 1.0f;

        Node() = default;

        Node(float2 pos, float energy_, float alpha_) :
                position{pos},
                energy{energy_},
                alpha{alpha_} {
        }
    };

    ecs::entity entity_;
    TimeLayer timer;
    float2 offset;
    float drain_speed = 1.0f;
    float segment_distance_max = 10.0f;
    float particles_per_second = 15.0f;

    Trail2D() = default;

    explicit Trail2D(ecs::entity entity) :
            entity_(entity) {

    }

    void track(ecs::entity e);

    void update_trail();

private:

    void update_position(float dt);

    void create_node(const float2& pos);

public:
    ecs::entity tracked_target_;
    float2 position_;
    float2 position_last_;
    std::vector<Node> nodes_;
    float particles_gen_ = 0.0f;
    bool auto_update_ = true;
};

class TrailRenderer2D : public Drawable2D<TrailRenderer2D> {
public:
    explicit TrailRenderer2D(ecs::entity target_) :
            target{target_} {

    }

    void draw() override;

    void updateMesh(const std::vector<Trail2D::Node>& nodes);

    [[nodiscard]]
    bool hitTest(float2 point) const override { return false; }

    [[nodiscard]]
    rect_f getBounds() const override { return rect_f{}; }

public:
    ecs::entity target{};
    std::vector<float2> vertices;
    float width = 20.0f;
    float minWidth = 5.0f;
};

void updateTrails();

}


