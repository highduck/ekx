#include "SceneFactory.hpp"

#include <ek/serialize/streams.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/log.h>
#include <ek/assert.h>
#include <ekx/app/localization.h>

struct res_sg res_sg;

void setup_res_sg(void) {
    struct res_sg* R = &res_sg;
    rr_man_t* rr = &R->rr;

    rr->names = R->names;
    rr->data = R->data;
    rr->max = sizeof(R->data) / sizeof(R->data[0]);
    rr->num = 1;
    rr->data_size = sizeof(R->data[0]);
}

namespace ek {

entity_t createNode2D(string_hash_t tag) {
    auto e = ecs::create<Node, Transform2D, WorldTransform2D>();
    ecs::get<Node>(e).tag = tag;
    return e;
}

entity_t createNode2D(entity_t parent, string_hash_t tag, int index) {
    auto e = createNode2D(tag);
    if (index == -1) {
        append(parent, e);
    } else if (index == 0) {
        prepend(parent, e);
    } else {
        // not implemented yet
        append(parent, e);
    }
    return e;
}

void sg_load(SGFile* out, const void* data, uint32_t size) {
    if (size > 0) {
        input_memory_stream input{data, size};
        IO io{input};
        io(*out);
    } else {
        log_error("SCENE LOAD: empty buffer");
    }
}

const SGNodeData* sg_get(const SGFile* sg, string_hash_t library_name) {
    // TODO: optimize access!
    for (auto& item: sg->library) {
        if (item.libraryName == library_name) {
            return &item;
        }
    }
    return nullptr;
}

void apply(entity_t entity, const SGNodeData* data) {
    if (data->movieTargetId >= 0) {
        ecs::add<MovieClipTargetIndex>(entity).key = data->movieTargetId;
    }

    {
        auto& transform = ecs::get<Transform2D>(entity);
        transform.setMatrix(data->matrix);
        transform.color.scale = color_vec4(data->color.scale);
        transform.color.offset = color_vec4(data->color.offset);
    }

    set_touchable(entity, data->touchable);
    set_visible(entity, data->visible);
    if (data->name) {
        // override
        set_tag(entity, data->name);
    }

    if (!data->dynamicText.empty()) {
        const auto& dynamicText = data->dynamicText[0];
        TextFormat format{dynamicText.font, dynamicText.size};
        format.alignment = dynamicText.alignment;
        format.leading = dynamicText.lineSpacing;
        format.wordWrap = dynamicText.wordWrap;
        format.layersCount = dynamicText.layers.size();
        if (format.layersCount > 4) {
            format.layersCount = 4;
        }
        for (int i = 0; i < format.layersCount; ++i) {
            auto& layer = dynamicText.layers[i];
            format.layers[i].color = layer.color;
            format.layers[i].offset = layer.offset;
            format.layers[i].blurRadius = layer.blurRadius;
            format.layers[i].blurIterations = layer.blurIterations;
            format.layers[i].strength = layer.strength;
        }

        auto* dtext = text2d_setup(entity);

        dtext->c_str = dynamicText.text.c_str();
        dtext->flags = ((dtext->flags >> 2) << 2);

        dtext->format = format;
        dtext->localized = is_localized(dynamicText.text.c_str());
        dtext->adjustsFontSizeToFitBounds = dtext->localized;
        dtext->rect = dynamicText.rect;
    }

    if (!data->movie.empty()) {
        auto& mov = ecs::add<MovieClip>(entity);
        mov.data = &data->movie[0];
        mov.fps = data->movie[0].fps;
    }

    Sprite2D* sprite = ecs::try_get<Sprite2D>(entity);
    NinePatch2D* ninePatch = ecs::try_get<NinePatch2D>(entity);

    if (data->sprite && !sprite) {
        if (rect_is_empty(data->scaleGrid)) {
            sprite = sprite2d_setup(entity);
            new Sprite2D();
            sprite->src = R_SPRITE(data->sprite);
        } else {
            ninePatch = ninepatch2d_setup(entity);
            ninePatch->src = R_SPRITE(data->sprite);
            ninePatch->scale_grid = data->scaleGrid;
        }
    }

    if (ninePatch) {
        ninePatch->scale = mat3x2_get_scale(data->matrix);
    }

    if (data->scissorsEnabled || data->hitAreaEnabled || data->boundsEnabled) {
        auto& bounds = ecs::add<Bounds2D>(entity);
        bounds.rect = data->boundingRect;
        if (data->scissorsEnabled) {
            bounds.flags |= BOUNDS_2D_SCISSORS;
        }
        if (data->hitAreaEnabled) {
            bounds.flags |= BOUNDS_2D_HIT_AREA;
        }
        if (data->boundsEnabled) {
            bounds.flags |= BOUNDS_2D_CULL;
        }
    }

    if (data->button) {
        ecs::add<Interactive, Button>(entity);
    }

    // TODO: remove ugly filters
//    if (!data->filters.empty()) {
//        auto& filters_comp = entity.reassign<UglyFilter2D>();
//        filters_comp.filters = data->filters;
//        ecs::replace_or_assign<node_filters_t>(entity);
//    }
}

entity_t create_and_merge(const SGFile* sg,
                          const SGNodeData* data,
                          const SGNodeData* over = nullptr) {
    auto entity = ecs::create<Node, Transform2D, WorldTransform2D>();
    if (data) {
        apply(entity, data);
    }
    if (over) {
        apply(entity, over);
    }
    if (data) {
        for (const auto& child: data->children) {
            auto child_entity = create_and_merge(sg, sg_get(sg, child.libraryName), &child);
            append_strict(entity, child_entity);
        }
    }

    return entity;
}

void extend_bounds(const SGFile* file, const SGNodeData& data, aabb2_t* boundsBuilder,
                   const mat3x2_t matrix) {
    sprite_t* spr = &REF_RESOLVE(res_sprite, data.sprite);
    if (spr->state & SPRITE_LOADED) {
        *boundsBuilder = aabb2_add_transformed_rect(*boundsBuilder, spr->rect, matrix);
    }
    for (const auto& child: data.children) {
        const auto& symbol = child.libraryName ? *sg_get(file, child.libraryName) : child;
        extend_bounds(file, symbol, boundsBuilder, mat3x2_mul(matrix, child.matrix));
    }
}

entity_t sg_create(string_hash_t library, string_hash_t name, entity_t parent) {
    entity_t result = NULL_ENTITY;
    R(SGFile) file_ref = R_SG(library);
    if (file_ref) {
        const SGFile* file = &REF_RESOLVE(res_sg, file_ref);
        const SGNodeData* data = sg_get(file, name);
        if (data) {
            result = create_and_merge(file, data);
            if (result.id && parent.id) {
                append_strict(parent, result);
            }
        } else {
            log_warn("SG Object (%s) not found in library %s", hsp_get(name), hsp_get(library));
        }
    } else {
        log_warn("SG not found: (%s)", hsp_get(library));
    }
    return result;
}

rect_t sg_get_bounds(string_hash_t library, string_hash_t name) {
    R(SGFile) file_ref = R_SG(library);
    if (file_ref) {
        SGFile* file = &REF_RESOLVE(res_sg, file_ref);
        const SGNodeData* data = sg_get(file, name);
        if (data) {
            aabb2_t bb = aabb2_empty();
            extend_bounds(file, *data, &bb, data->matrix);
            return aabb2_get_rect(bb);
        }
    }
    return {};
}

}