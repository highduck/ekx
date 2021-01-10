#include "Trail2D.hpp"

#include <ek/scenex/utility/scene_management.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/easing.hpp>

namespace ek {

void Trail2D::track(ecs::entity e) {
    tracked_target_ = e;
    if (tracked_target_) {
        position_ = position_last_ = Transform2D::localToLocal(tracked_target_, entity_, offset);
        nodes_.clear();

        // tail
        nodes_.emplace_back(position_last_, 0.0f, 1.0f);

        // head
        nodes_.emplace_back(position_last_, 1.0f, 1.0f);
    }
}

void Trail2D::update() {
    float dt = timer->dt;
    auto head_node = nodes_[nodes_.size() - 1];
    if (tracked_target_) {
        if (Transform2D::fastLocalToLocal(tracked_target_, entity_, offset, position_)) {
            update_position();
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

void Trail2D::update_position() {
    //nextPosition.x += FastMath.Range(-10f, 10f);
    //nextPosition.y += FastMath.Range(-10f, 10f);
    auto pos = position_last_;
    auto direction = position_ - position_last_;
    auto distance = length(direction);

    direction = normalize(direction);

    auto head = nodes_.back();

//    auto limit = 100u;
    while (distance >= segment_distance_max) { // && limit > 0u
        distance -= segment_distance_max;
        pos += direction * segment_distance_max;
        head.position = pos;
        nodes_.push_back(head);
//        --limit;
    }

//    if (ps != null) {
//        spawnParticles(pos, direction, dt);
//    }

    position_last_ = pos;
    nodes_.back().position = position_;
}

void Trail2D::create_node(float2 pos) {
    auto& head = nodes_.back();
    //if (headNode.power < 1.0f)
    //{
    //	headNode.growPower(1.0f / growSpeed);
    //}

    head.position = pos;

    // update head position and duplicate to new
    nodes_.push_back(head);
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

void Trail2D::updateAll() {
    ecs::view<Trail2D>().each([](Trail2D& trail) {
        trail.update();
    });
}

void TrailRenderer2D::draw() {
    auto& trail = target.get<Trail2D>();
    auto& nodes = trail.nodes_;
    updateMesh(trail.nodes_);

    const auto columns = static_cast<int>(nodes.size());
    const auto quads = columns - 1;
    if (quads <= 0) {
        return;
    }
    const auto* spr = sprite.get();
    if (spr == nullptr) {
        return;
    }
    const auto* texture = spr->texture.get();
    if (texture == nullptr) {
        return;
    }

    auto& drawer = draw2d::state;

    drawer.set_texture(texture);
    drawer.allocTriangles(columns * 2, quads * 6);

    int v = 0;
    int node_idx = 0;

    const auto co = drawer.color.offset;
    const auto cm = drawer.color.scale;
    const auto texCoordU = spr->tex.center_x();
    const auto texCoordV0 = spr->tex.y;
    const auto texCoordV1 = spr->tex.bottom();
    const auto m = drawer.matrix;
    auto* ptr = drawer.vertexDataPos_;

    for (int i = 0; i < columns; ++i) {
        const auto cm0 = cm.scaleAlpha(nodes[node_idx].energy);
        const auto v1 = vertices[v++];
        const auto v2 = vertices[v++];
        ptr->position = m.transform(v1.x, v1.y);
        ptr->uv.x = texCoordU;
        ptr->uv.y = texCoordV0;
        ptr->cm = cm0;
        ptr->co = co;
        ++ptr;
        ptr->position = m.transform(v2.x, v2.y);
        ptr->uv.x = texCoordU;
        ptr->uv.y = texCoordV1;
        ptr->cm = cm0;
        ptr->co = co;
        ++ptr;
        ++node_idx;
    }

    v = drawer.baseVertex_;
    uint16_t* indices = drawer.indexDataPos_;
    for (int i = 0; i < quads; ++i) {
        *(indices++) = v;
        *(indices++) = v + 2;
        *(indices++) = v + 3;
        *(indices++) = v + 3;
        *(indices++) = v + 1;
        *(indices++) = v;
        v += 2;
    }
}
}