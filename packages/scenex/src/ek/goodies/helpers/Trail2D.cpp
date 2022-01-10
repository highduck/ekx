#include "Trail2D.hpp"

#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/canvas.h>


namespace ek {

void Trail2D::update(const mat3x2_t m) {
    float dt = timer->dt;
    auto scale2 = mat2_get_scale(m.rot);
    scale = fminf(scale2.x, scale2.y);
    update_position(vec2_transform(offset, m));

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

void Trail2D::update_position(vec2_t newPosition) {

    if (!initialized) {
        initialized = true;
        lastPosition = newPosition;
        return;
    }

    //nextPosition.x += FastMath.Range(-10f, 10f);
    //nextPosition.y += FastMath.Range(-10f, 10f);
//    auto pos = lastPosition;
    const auto distanceSqr = length_sqr_vec2(newPosition - lastPosition);

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

    canvas_set_image(image);
    canvas_triangles(columns * 2, quads * 6);

    auto node_idx = trail.nodes.first;

    const auto co = canvas.color[0].offset;
    const auto cm = canvas.color[0].scale;
    auto texCoordU0 = RECT_CENTER_X(spr->tex);
    auto texCoordV0 = spr->tex.y;
    auto texCoordU1 = RECT_CENTER_X(spr->tex);
    auto texCoordV1 = RECT_B(spr->tex);
    if(spr->rotated) {
        texCoordU0 = spr->tex.x;
        texCoordV0 = RECT_CENTER_Y(spr->tex);
        texCoordU1 = RECT_R(spr->tex);
        texCoordV1 = RECT_CENTER_Y(spr->tex);
    }

    //const auto m = drawer.matrix;
    //drawer.matrix.set
    ek_vertex2d* ptr = canvas.vertex_it;

    // we could generate vertices right into destination buffer :)
    for (int i = 0; i < columns; ++i) {
        const auto p = nodeArray[node_idx].position;
        vec2_t perp = {};
        if (i > 0/* node_idx > begin */) {
            perp = normalize_vec2(nodeArray[node_idx - 1].position - p);
            if (i + 1 < columns) {
                perp = normalize_vec2(lerp_vec2(perp, normalize_vec2(p - nodeArray[node_idx + 1].position), 0.5f));
            }
        } else if (i + 1 < columns) {
            perp = normalize_vec2(p - nodeArray[node_idx + 1].position);
        }
        perp = perp_vec2(perp);

        const auto energy = nodeArray[node_idx].energy;
        const auto easedEnergy = ease_p2_out(energy);
        const auto r = lerp_f32(minWidth, width, easedEnergy);
        const auto nodeScale = nodeArray[node_idx].scale;
        perp *= nodeScale * r;

        const color_t cm0 = color_alpha_scale_f(cm, energy);
        ptr->x = p.x - perp.x;
        ptr->y = p.y - perp.y;
        ptr->u = texCoordU0;
        ptr->v = texCoordV0;
        ptr->cm = cm0.value;
        ptr->co = co.value;
        ++ptr;
        ptr->x = p.x + perp.x;
        ptr->y = p.y + perp.y;
        ptr->u = texCoordU1;
        ptr->v = texCoordV1;
        ptr->cm = cm0.value;
        ptr->co = co.value;
        ++ptr;
        ++node_idx;
    }

    {
        uint16_t v = canvas.vertex_base;
        uint16_t* indices = canvas.index_it;
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