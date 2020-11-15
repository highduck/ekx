#include "trail_script.hpp"

#include <ek/scenex/utility/scene_management.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/easing.hpp>

namespace ek {

void trail_script::draw() {
//
//    if (true) {
//        auto& drawer = resolve<Drawer>();
//        for (auto& node : nodes_) {
//            auto size = 40.0f * node.energy;
//
//            drawer.quad(node.position.x - size * 0.5f, node.position.y - size * 0.5f, size, size,
//                        lerp(0x00FFFFFF_argb, 0xFFFFFFFF_argb, 0.5f * node.alpha));
//        }
//    }
//
//    if (true) {
//        auto& drawer = resolve<Drawer>();
//        int v = 0;
//        for (int i = 0; i < nodes_.size(); ++i) {
//            auto v0 = vertexList_[v];
//            auto v1 = vertexList_[v + 1];
//
//            drawer.quad(v0.x - 4, v0.y - 4, 8, 8, 0xFF0000_rgb);
//            drawer.quad(v1.x - 4, v1.y - 4, 8, 8, 0x0000FF_rgb);
//            v += 2;
//        }
//    }

    const auto columns = static_cast<int>(nodes_.size());
    const auto quads = columns - 1;
    if (quads > 0) {
        draw2d::state.set_empty_texture();
        draw2d::prepare();
        draw2d::triangles(columns * 2, quads * 6);

        int v = 0;
        int node_idx = 0;

        for (int i = 0; i < columns; ++i) {
            const auto e0 = nodes_[node_idx].energy;
            const auto cm0 = draw2d::state.calc_vertex_color_multiplier(
                    argb32_t{1.0f, 1.0f, 1.0f, e0}
            );
            const auto co = draw2d::state.vertex_color_offset;
            const auto v0 = vertex_list_[v];
            const auto v1 = vertex_list_[v + 1];
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

void trail_script::update(float dt) {
    update_trail(dt);
}

void trail_script::track(ecs::entity e) {
    tracked_target_ = e;
    if (tracked_target_) {
        auto position_global = local_to_global(tracked_target_, offset);
        auto position_trail_space = global_to_local(entity_, position_global);
        position_ = position_last_ = position_trail_space;

        nodes_.clear();

        // tail
        nodes_.emplace_back(position_last_,
                            0.0f,
                            1.0f
        );

        // head
        nodes_.emplace_back(position_last_,
                            1.0f,
                            1.0f
        );
    }
}

void trail_script::update_trail(float dt) {
    auto head_node = nodes_[nodes_.size() - 1];
    if (tracked_target_) {
        auto position_global = local_to_global(tracked_target_, offset);
        auto position_trail_space = global_to_local(entity_, position_global);
        update_position(position_trail_space, dt);
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

    // todo:
    update_mesh();
}

void trail_script::update_position(const float2& position_next, float dt) {
    //nextPosition.x += FastMath.Range(-10f, 10f);
    //nextPosition.y += FastMath.Range(-10f, 10f);
    auto pos = position_last_;
    auto direction = position_next - position_last_;
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

    position_ = position_next;
    nodes_[nodes_.size() - 1].position = position_;
}

void trail_script::create_node(const float2& pos) {
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

void trail_script::update_mesh() {
    auto total = nodes_.size();
    auto count = total * 2;

    if (vertex_list_.size() < count) {
        vertex_list_.resize(count);
    }
    int vi = 0;
    float2 perp;
    for (int i = 0; i < total; ++i) {
        const float2 p = nodes_[i].position;
        if (i > 0) {
            float2 perp0 = normalize(nodes_[i - 1].position - p);
            if (i + 1 < total) {
                perp0 = normalize(lerp(perp0, normalize(p - nodes_[i + 1].position), 0.5f));
            }
            perp = perpendicular(perp0);
        }

//        if (i == total - 1) {
//            perp = YUnit2;
//        }

        auto energy = easing::P2_OUT.calculate(nodes_[i].energy);
        auto r = min_width + (width - min_width) * energy;
        auto off = perp * r;
        vertex_list_[vi] = p - off;
        vertex_list_[vi + 1] = p + off;
        vi += 2;
        //m -= dm;
    }
//    view.SetPositions(vertices, count);

}

}