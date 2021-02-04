#include "Trail2D.hpp"

#include <ek/scenex/utility/scene_management.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/easing.hpp>

namespace ek {

void Trail2D::update(const matrix_2d& m) {
    float dt = timer->dt;
    scale = length(m.scale());
    update_position(m.transform(offset));

    for (uint32_t i = nodes.first; i < nodes.end; ++i) {
        auto& node = nodes.data[i];
        node.energy -= dt * drain_speed;
        if (node.energy <= 0.0f) {
            node.energy = 0.0f;
            if (i == nodes.first) {
                nodes.erase_front();
            }
        }
    }
}

void Trail2D::update_position(float2 newPosition) {

    if (!initialized) {
        initialized = true;
        lastPosition = newPosition;
        return;
    }

    //nextPosition.x += FastMath.Range(-10f, 10f);
    //nextPosition.y += FastMath.Range(-10f, 10f);
//    auto pos = lastPosition;
    const auto distanceSqr = length_sqr(newPosition - lastPosition);

//    direction *= 1.0f / distance;

//    auto headCopy = nodes.back();
    if (distanceSqr >= segment_distance_max * segment_distance_max) {
        lastPosition = newPosition;
        //headCopy.position = newPosition;
        Node newNode{};
        newNode.scale = scale;
        nodes.push_back(newNode);
    }

    if (nodes.size() > 0) {
        nodes.back().position = newPosition;
        nodes.back().scale = scale;
    } else {
        //lastPosition = newPosition;
    }
}

void Trail2D::updateAll() {
    const auto* w = ecs::the_world;
    auto* trails = ecs::tpl_world_storage<Trail2D>(w);
    const auto count = trails->component.count;
    for (uint32_t i = 1; i < count; ++i) {
        auto e = trails->component.handleToEntity.get(i);
        const auto& m = w->get<WorldTransform2D>(e).matrix;
        trails->data[i].update(m);
    }
}

void TrailRenderer2D::draw() {
    auto& trail = w->get<Trail2D>(target.index);
    auto& nodeArray = trail.nodes.data;

    const auto columns = static_cast<int>(trail.nodes.size());
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

    auto node_idx = trail.nodes.first;

    const auto co = drawer.color.offset;
    const auto cm = drawer.color.scale;
    const auto texCoordU = spr->tex.center_x();
    const auto texCoordV0 = spr->tex.y;
    const auto texCoordV1 = spr->tex.bottom();
    //const auto m = drawer.matrix;
    //drawer.matrix.set
    auto* ptr = drawer.vertexDataPos_;

    // we could generate vertices right into destination buffer :)
    for (int i = 0; i < columns; ++i) {

//        const auto v1 = vertices[v++];
//        const auto v2 = vertices[v++];
        const float2 p = nodeArray[node_idx].position;
        float2 perp{};
        if (i > 0/* node_idx > begin */) {
            perp = normalize_2f(nodeArray[node_idx - 1].position - p);
            if (i + 1 < columns) {
                perp = normalize_2f(lerp(perp, normalize_2f(p - nodeArray[node_idx + 1].position), 0.5f));
            }
        } else if (i + 1 < columns) {
            perp = normalize_2f(p - nodeArray[node_idx + 1].position);
        }
        perp = perpendicular(perp);

        const auto energy = nodeArray[node_idx].energy;
        perp *= nodeArray[node_idx].scale * math::lerp(minWidth, width, easing::P2_OUT.calculate(energy));

        const auto cm0 = cm.scaleAlpha(energy);
        ptr->position = p - perp;
        ptr->uv.x = texCoordU;
        ptr->uv.y = texCoordV0;
        ptr->cm = cm0;
        ptr->co = co;
        ++ptr;
        ptr->position = p + perp;
        ptr->uv.x = texCoordU;
        ptr->uv.y = texCoordV1;
        ptr->cm = cm0;
        ptr->co = co;
        ++ptr;
        ++node_idx;
    }

    {
        uint16_t v = drawer.baseVertex_;
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
}