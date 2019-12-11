#include "sg_factory.h"

#include <ek/scenex/2d/sprite.hpp>
#include <ek/scenex/components/node_t.h>
#include <ek/scenex/components/transform_2d.h>
#include <ek/scenex/components/display_2d.h>
#include <ek/scenex/components/name_t.h>
#include <ek/scenex/components/movie_t.h>
#include <ek/scenex/components/button_t.h>
#include <ek/scenex/components/interactive_t.h>
#include <ek/scenex/components/node_filters.h>
#include <ek/util/logger.hpp>

namespace ek {

namespace {

using asset_ref = asset_t<sg_file>;

void apply(ecs::entity entity, const sg_node_data* data, asset_ref asset) {
    ecs::get<name_t>(entity).name = data->name;
    ecs::get<movie_target_keys>(entity) = {data->animationKey, data->layerKey};

    auto& transform = ecs::get<transform_2d>(entity);
    transform.matrix.tx = data->matrix.tx;
    transform.matrix.ty = data->matrix.ty;
    transform.skew = data->matrix.skew();
    transform.scale = data->matrix.scale();
    transform.colorMultiplier = argb32_t(data->color.multiplier);
    transform.colorOffset = argb32_t(data->color.offset);

    auto& node_state = ecs::get<node_state_t>(entity);
    node_state.touchable = data->touchable;
    node_state.visible = data->visible;

    if (data->dynamicText.has_value()) {
        const auto& dynamicText = data->dynamicText.value();
        text_format_t format{dynamicText.face, dynamicText.size};
        format.alignment = dynamicText.alignment;
        format.lineSpacing = dynamicText.line_spacing;
        format.lineHeight = dynamicText.line_height;
        format.shadow = false;
        format.color = dynamicText.color;

        auto& display = ecs::get_or_create<display_2d>(entity);
        auto dtext = std::make_unique<drawable_text>(dynamicText.text, format);
        dtext->rect = dynamicText.rect;
        display.drawable = std::move(dtext);
    }

    if (data->movie.has_value()) {
        auto& mov = ecs::assign<movie_t>(entity);
        if (asset) {
            mov.library_asset = asset;
            mov.movie_data_symbol = data->libraryName;
        } else {
            mov.data = &(data->movie.value());
        }
    }

    drawable_sprite* sprite = nullptr;
    if (ecs::has<display_2d>(entity)) {
        auto& display = ecs::get<display_2d>(entity);
        if (display.drawable && display.drawable->get_type_id() == drawable_sprite::type_id) {
            sprite = static_cast<drawable_sprite*>(display.drawable.get());
        }
    }

    if (!data->sprite.empty() && !sprite) {
        auto& display = ecs::get_or_create<display_2d>(entity);
        sprite = new drawable_sprite(data->sprite, data->scaleGrid);
        display.drawable.reset(sprite);
    }

    if (sprite && sprite->scale_grid_mode) {
        sprite->scale = data->matrix.scale();
    }

    if (!data->clipRect.empty()) {
        ecs::replace_or_assign<scissors_2d>(entity, data->clipRect);
    }

    if (!data->hitRect.empty()) {
        ecs::replace_or_assign<hit_area_2d>(entity, data->hitRect);
    }

    if (data->button) {
        ecs::replace_or_assign<interactive_t>(entity);
        ecs::replace_or_assign<button_t>(entity);
    }

    if (!data->filters.empty()) {
        auto& filters_comp = ecs::replace_or_assign<node_filters_t>(entity);
        filters_comp.filters = data->filters;
//        ecs::replace_or_assign<node_filters_t>(entity);
    }
}

ecs::entity create_and_merge(const sg_file& sg, asset_ref asset,
                             const sg_node_data* data,
                             const sg_node_data* over = nullptr) {
    auto entity = ecs::create<node_t, node_state_t, transform_2d, name_t, movie_target_keys>();
    if (data) {
        apply(entity, data, asset);
    }
    if (over) {
        apply(entity, over, asset);
    }
    if (data) {
        for (const auto& child : data->children) {
            auto child_entity = create_and_merge(sg, asset, sg.get(child.libraryName), &child);
            append_strict(entity, child_entity);
        }
    }

    return entity;
}

void extend_bounds(const sg_file& file, const sg_node_data& data, rect_f& bounds, const matrix_2d& matrix) {
    if (!data.sprite.empty()) {
        asset_t<sprite_t> spr{data.sprite};
        if (spr) {
            bounds = combine(bounds, points_box(
                    matrix.transform(spr->rect.position),
                    matrix.transform(spr->rect.right_bottom())
            ));
        }
    }
    for (const auto& child : data.children) {
        const auto& symbol = child.libraryName.empty() ? child : *file.get(child.libraryName);
        extend_bounds(file, symbol, bounds, matrix * child.matrix);
    }
}

}

ecs::entity sg_create(const std::string& library, const std::string& name) {
    ecs::entity result;
    asset_t<sg_file> file{library};
    if (file) {
        const sg_node_data* data = file->get(name);
        if (data) {
            result = create_and_merge(*file, file, data);
        } else {
            EK_WARN("SG Object %s not found in library %s", name.c_str(), library.c_str());
        }
    } else {
        EK_WARN("SG not found: %s", library.c_str());
    }
    return result;
}

rect_f sg_get_bounds(const std::string& library, const std::string& name) {
    asset_t<sg_file> file{library};
    if (file) {
        const sg_node_data* data = file->get(name);
        if (data) {
            rect_f rc{};
            extend_bounds(*file, *data, rc, data->matrix);
            return rc;
        }
    }
    return {};
}

}