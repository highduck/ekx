#include "Trail2D.hpp"

#include <ek/scenex/utility/scene_management.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/easing.hpp>

namespace ek {

void Trail2D::track(ecs::entity e) {
    tracked_target_ = e;
    if (tracked_target_) {
        Transform2D::updateLocalMatrixSubTree(tracked_target_, entity_);
        position_ = position_last_ = Transform2D::localToLocal(tracked_target_, entity_, offset);
        nodes_.clear();

        // tail
        nodes_.emplace_back(position_last_,0.0f,1.0f);

        // head
        nodes_.emplace_back(position_last_,1.0f,1.0f);
    }
}

void Trail2D::update_trail() {
    float dt = timer->dt;
    auto head_node = nodes_[nodes_.size() - 1];
    if (tracked_target_) {
        if (Transform2D::fastLocalToLocal(tracked_target_, entity_, offset, position_)) {
            update_position(dt);
        }
    } else {
        head_node.energy -= dt;
        if (head_node.energy < 0.0f) {
            head_node.energy = 0.0f;
        }
        if (nodes_.size() == 1 && head_node.energy <= 0.0f) {
            // todo: auto-destroy
//            view.Dispose();
//            view = null;
            return;
        }
    }

    int i = 0;
    while (i < nodes_.size() - 1) {
        auto& node = nodes_[i];
        node.energy -= dt * drain_speed;
        if (node.energy <= 0.0f) {
            node.energy = 0.0f;
            if (nodes_[i + 1].energy <= 0.0f) {
                nodes_.erase(nodes_.begin() + i);
            } else {
                ++i;
            }
        } else {
            ++i;
        }
    }
}

void Trail2D::update_position(float dt) {
    //nextPosition.x += FastMath.Range(-10f, 10f);
    //nextPosition.y += FastMath.Range(-10f, 10f);
    auto pos = position_last_;
    auto direction = position_ - position_last_;
    auto distance = length(direction);

    direction = normalize(direction);

//    auto limit = 100u;
    while (distance >= segment_distance_max) { // && limit > 0u
        distance -= segment_distance_max;
        pos += direction * segment_distance_max;
        create_node(pos);
//        --limit;
    }

//    if (ps != null) {
//        spawnParticles(pos, direction, dt);
//    }

    position_last_ = pos;
    nodes_[nodes_.size() - 1].position = position_;
}

void Trail2D::create_node(const float2& pos) {
    auto head_node = nodes_[nodes_.size() - 1];
    //if (headNode.power < 1.0f)
    //{
    //	headNode.growPower(1.0f / growSpeed);
    //}

    nodes_[nodes_.size() - 1] = {
            pos,
            head_node.energy,
            head_node.alpha
    };
    nodes_.push_back(head_node);
}

void TrailRenderer2D::updateMesh(const std::vector<Trail2D::Node>& nodes) {
    auto total = nodes.size();
    auto count = total * 2;

    if (vertices.size() < count) {
        vertices.resize(count);
    }
    int vi = 0;

    for (int i = 0; i < total; ++i) {
        const float2 p = nodes[i].position;
        float2 perp{};
        if (i > 0) {
            perp = normalize(nodes[i - 1].position - p);
            if (i + 1 < total) {
                perp = normalize(lerp(perp, normalize(p - nodes[i + 1].position), 0.5f));
            }
            perp = perpendicular(perp);
        }

//        if (i == total - 1) {
//            perp = YUnit2;
//        }

        auto energy = easing::P2_OUT.calculate(nodes[i].energy);
        perp *= nodes[i].scale * math::lerp(minWidth, width, energy);
        vertices[vi] = p - perp;
        vertices[vi + 1] = p + perp;
        vi += 2;
        //m -= dm;
    }
//    view.SetPositions(vertices, count);

}

void updateTrails() {
    ecs::view<Trail2D>().each([](Trail2D& trail) {
        trail.update_trail();
    });
}

void TrailRenderer2D::draw() {
    auto& trail = target.get<Trail2D>();
    auto& nodes = trail.nodes_;
    updateMesh(trail.nodes_);

    const auto columns = static_cast<int>(nodes.size());
    const auto quads = columns - 1;
    if (quads > 0) {
        draw2d::state.set_empty_texture();
        draw2d::prepare();
        draw2d::triangles(columns * 2, quads * 6);

        int v = 0;
        int node_idx = 0;

        for (int i = 0; i < columns; ++i) {
            const auto e0 = nodes[node_idx].energy;
            const auto cm0 = draw2d::state.calc_vertex_color_multiplier(
                    argb32_t{1.0f, 1.0f, 1.0f, e0}
            );
            const auto co = draw2d::state.vertex_color_offset;
            const auto v0 = vertices[v];
            const auto v1 = vertices[v + 1];
            draw2d::write_vertex(v0.x, v0.y, 0.0f, 0.0f, cm0, co);
            draw2d::write_vertex(v1.x, v1.y, 0.0f, 1.0f, cm0, co);

            if (i < quads) {
                draw2d::write_indices_quad(0, 2, 3, 1, v);
            }

            v += 2;
            ++node_idx;
        }
    }
}
}