#include "Trail2D.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/draw2d/drawer.hpp>
#include <ek/math/Easings.hpp>

namespace ek {

void Trail2D::update(const Matrix3x2f& m) {
    float dt = timer->dt;
    auto scale2 = m.scale();
    scale = fminf(scale2.x, scale2.y);
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

void Trail2D::update_position(Vec2f newPosition) {

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
    const auto* w = &ecs::the_world;
    auto* trails = w->getStorage<Trail2D>();
    const auto count = trails->component.count();
    for (uint32_t i = 1; i < count; ++i) {
        auto e = trails->component.handleToEntity.get(i);
        const auto& m = w->get<WorldTransform2D>(e).matrix;
        trails->data[i].update(m);
    }
}

void TrailRenderer2D::draw() {
    auto& trail = w->get<Trail2D>(target.index);
    auto& nodeArray = trail.nodes.data;

    const uint32_t columns = trail.nodes.size();
    if(columns < 2) {
        return;
    }
    const auto* spr = sprite.get();
    if (spr == nullptr) {
        return;
    }
    const sg_image image = ek_ref_content(sg_image, spr->image_id);
    if (image.id == SG_INVALID_ID) {
        return;
    }

    const uint32_t quads = columns - 1;
    auto& drawer = draw2d::state;

    drawer.set_image(image);
    drawer.allocTriangles(columns * 2, quads * 6);

    auto node_idx = trail.nodes.first;

    const auto co = drawer.color[0].offset;
    const auto cm = drawer.color[0].scale;
    auto texCoordU0 = spr->tex.center_x();
    auto texCoordV0 = spr->tex.y;
    auto texCoordU1 = spr->tex.center_x();
    auto texCoordV1 = spr->tex.bottom();
    if(spr->rotated) {
        texCoordU0 = spr->tex.x;
        texCoordV0 = spr->tex.center_y();
        texCoordU1 = spr->tex.right();
        texCoordV1 = spr->tex.center_y();
    }

    //const auto m = drawer.matrix;
    //drawer.matrix.set
    auto* ptr = (draw2d::Vertex2D*)ek_canvas_.vertex_it;

    // we could generate vertices right into destination buffer :)
    for (int i = 0; i < columns; ++i) {
        const Vec2f p = nodeArray[node_idx].position;
        Vec2f perp{};
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
        const auto easedEnergy = easing::P2_OUT.calculate(energy);
        const auto r = Math::lerp(minWidth, width, easedEnergy);
        const auto nodeScale = nodeArray[node_idx].scale;
        perp *= nodeScale * r;

        const auto cm0 = cm.scaleAlpha(energy);
        ptr->position = p - perp;
        ptr->uv.x = texCoordU0;
        ptr->uv.y = texCoordV0;
        ptr->cm = cm0;
        ptr->co = co;
        ++ptr;
        ptr->position = p + perp;
        ptr->uv.x = texCoordU1;
        ptr->uv.y = texCoordV1;
        ptr->cm = cm0;
        ptr->co = co;
        ++ptr;
        ++node_idx;
    }

    {
        uint16_t v = ek_canvas_.vertex_base;
        uint16_t* indices = ek_canvas_.index_it;
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