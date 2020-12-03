#include "sg_factory.hpp"

#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/2d/UglyFilter2D.hpp>
#include <ek/util/logger.hpp>
#include <ek/math/bounds_builder.hpp>
#include <ek/Localization.hpp>

namespace ek {

namespace {

using asset_ref = Res<sg_file>;

void apply(ecs::entity entity, const sg_node_data* data, asset_ref asset) {
    auto& node = ecs::get<Node>(entity);
    node.name = data->name;
    if (data->movieTargetId >= 0) {
        ecs::get_or_create<movie_target_keys>(entity) = {data->movieTargetId};
    }

    auto& transform = ecs::get<Transform2D>(entity);
    transform.position = data->matrix.position();
    transform.skew = data->matrix.skew();
    transform.scale = data->matrix.scale();
    transform.color.scale = argb32_t(data->color.scale);
    transform.color.offset = argb32_t(data->color.offset);

    node.setTouchable(data->touchable);
    node.setVisible(data->visible);

    if (data->dynamicText.has_value()) {
        const auto& dynamicText = data->dynamicText.value();
        TextFormat format{dynamicText.font.c_str(), dynamicText.size};
        format.alignment = dynamicText.alignment;
        format.leading = dynamicText.line_spacing;
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

        auto& display = ecs::get_or_create<Display2D>(entity);
        auto dtext = std::make_unique<Text2D>(dynamicText.text, format);
        dtext->localize = Localization::instance.has(dynamicText.text);
        dtext->rect = dynamicText.rect;
        display.drawable = std::move(dtext);
    }

    if (data->movie.has_value()) {
        auto& mov = ecs::assign<MovieClip>(entity);
        if (asset) {
            mov.library_asset = asset;
            mov.movie_data_symbol = data->libraryName;
        } else {
            mov.data = &(data->movie.value());
        }
        mov.fps = data->movie->fps;
    }

    Sprite2D* sprite = nullptr;
    if (ecs::has<Display2D>(entity)) {
        auto& display = ecs::get<Display2D>(entity);
        if (display.is<Sprite2D>()) {
            sprite = display.get<Sprite2D>();
        }
    }

    if (!data->sprite.empty() && !sprite) {
        auto& display = ecs::get_or_create<Display2D>(entity);
        sprite = new Sprite2D(data->sprite, data->scaleGrid);
        display.drawable.reset(sprite);
    }

    if (sprite && sprite->scale_grid_mode) {
        sprite->scale = data->matrix.scale();
    }

    if (data->scissorsEnabled || data->hitAreaEnabled || data->boundsEnabled) {
        auto& bounds = ecs::reassign<Bounds2D>(entity, data->boundingRect);
        bounds.scissors = data->scissorsEnabled;
        bounds.hitArea = data->hitAreaEnabled;
        bounds.culling = data->boundsEnabled;
    }

    if (data->button) {
        ecs::reassign<Interactive>(entity);
        ecs::reassign<Button>(entity);
    }

    if (!data->filters.empty()) {
        auto& filters_comp = ecs::reassign<UglyFilter2D>(entity);
        filters_comp.filters = data->filters;
//        ecs::replace_or_assign<node_filters_t>(entity);
    }
}

ecs::entity create_and_merge(const sg_file& sg, asset_ref asset,
                             const sg_node_data* data,
                             const sg_node_data* over = nullptr) {
    auto entity = ecs::create<Node, Transform2D>();
    if (data) {
        apply(entity, data, asset);
    }
    if (over) {
        apply(entity, over, asset);
    }
    if (data) {
        for (const auto& child : data->children) {
            auto child_entity = create_and_merge(sg, asset, sg.get(child.libraryName), &child);
            appendStrict(entity, child_entity);
        }
    }

    return entity;
}

void extend_bounds(const sg_file& file, const sg_node_data& data, bounds_builder_2f& boundsBuilder,
                   const matrix_2d& matrix) {
    const Res<Sprite> spr{data.sprite};
    if (spr) {
        boundsBuilder.add(spr->rect, matrix);
    }
    for (const auto& child : data.children) {
        const auto& symbol = child.libraryName.empty() ? child : *file.get(child.libraryName);
        extend_bounds(file, symbol, boundsBuilder, matrix * child.matrix);
    }
}

}

ecs::entity sg_create(const std::string& library, const std::string& name) {
    ecs::entity result;
    Res<sg_file> file{library};
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
    Res<sg_file> file{library};
    if (file) {
        const sg_node_data* data = file->get(name);
        if (data) {
            bounds_builder_2f bb{};
            extend_bounds(*file, *data, bb, data->matrix);
            return bb.rect();
        }
    }
    return {};
}

}