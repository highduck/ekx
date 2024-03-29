#include "SceneFactory.hpp"
#include <ek/scenex/data/SGFile.hpp>

#include <ek/serialize/streams.hpp>
#include <ek/scenex/2d/Sprite.hpp>
#include <ek/scenex/base/Node.hpp>
#include <ek/scenex/2d/Transform2D.hpp>
#include <ek/scenex/2d/Display2D.hpp>
#include <ek/scenex/2d/MovieClip.hpp>
#include <ek/scenex/2d/Button.hpp>
#include <ek/scenex/base/Interactive.hpp>
#include <ek/scenex/2d/UglyFilter2D.hpp>
#include <ek/debug.hpp>
#include <ek/util/Res.hpp>
#include <ek/math/bounds_builder.hpp>
#include <ek/Localization.hpp>

namespace ek {

ecs::EntityApi createNode2D(const char* name) {
    auto e = ecs::create<Node, Transform2D, WorldTransform2D>();
    if (name) {
        e.assign<NodeName>(name);
    }
    return e;
}

ecs::EntityApi createNode2D(ecs::EntityApi parent, const char* name, int index) {
    auto e = createNode2D(name);
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

SGFile* sg_load(const void* data, uint32_t size) {
    SGFile* sg = nullptr;

    if (size > 0) {
        input_memory_stream input{data, size};
        IO io{input};

        sg = new SGFile();
        io(*sg);
    } else {
        EK_ERROR("SCENE LOAD: empty buffer");
    }

    return sg;
}

const SGNodeData* sg_get(const SGFile& sg, const std::string& library_name) {
    // TODO: optimize access!
    for (auto& item: sg.library) {
        if (item.libraryName == library_name) {
            return &item;
        }
    }
    return nullptr;
}

using SGFileRes = Res<SGFile>;

void apply(ecs::EntityApi entity, const SGNodeData* data, SGFileRes asset) {
    if (!data->name.empty()) {
        entity.get_or_create<NodeName>().name = data->name;
    }

    if (data->movieTargetId >= 0) {
        entity.get_or_create<MovieClipTargetIndex>() = {data->movieTargetId};
    }

    {
        auto& transform = entity.get<Transform2D>();
        transform.setMatrix(data->matrix);
        transform.color.scale = argb32_t(data->color.scale);
        transform.color.offset = argb32_t(data->color.offset);
    }

    {
        auto& node = entity.get<Node>();
        node.setTouchable(data->touchable);
        node.setVisible(data->visible);
    }

    if (data->dynamicText.has_value()) {
        const auto& dynamicText = data->dynamicText.value();
        TextFormat format{dynamicText.font.c_str(), dynamicText.size};
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

        auto& display = entity.get_or_create<Display2D>();
        auto dtext = std::make_unique<Text2D>(dynamicText.text, format);
        dtext->localize = Localization::instance.has(dynamicText.text.c_str());
        dtext->adjustsFontSizeToFitBounds = dtext->localize;
        dtext->rect = dynamicText.rect;
        display.drawable = std::move(dtext);
    }

    if (data->movie.has_value()) {
        auto& mov = entity.assign<MovieClip>();
        if (asset) {
            mov.library_asset = asset;
            mov.movie_data_symbol = data->libraryName;
        } else {
            mov.data = &(data->movie.value());
        }
        mov.fps = data->movie->fps;
    }

    auto* display = entity.tryGet<Display2D>();
    Sprite2D* sprite = nullptr;
    NinePatch2D* ninePatch = nullptr;
    if (display) {
        sprite = display->tryGet<Sprite2D>();
        ninePatch = display->tryGet<NinePatch2D>();
    }

    if (!data->sprite.empty() && !sprite) {
        if (!display) {
            display = &entity.assign<Display2D>();
        }
        if (data->scaleGrid.empty()) {
            sprite = new Sprite2D(data->sprite);
            display->drawable.reset(sprite);
        } else {
            ninePatch = new NinePatch2D(data->sprite, data->scaleGrid);
            display->drawable.reset(ninePatch);
        }
    }

    if (ninePatch) {
        ninePatch->scale = data->matrix.scale();
    }

    if (data->scissorsEnabled || data->hitAreaEnabled || data->boundsEnabled) {
        auto& bounds = entity.reassign<Bounds2D>(data->boundingRect);
        bounds.scissors = data->scissorsEnabled;
        bounds.hitArea = data->hitAreaEnabled;
        bounds.culling = data->boundsEnabled;
    }

    if (data->button) {
        entity.reassign<Interactive>();
        entity.reassign<Button>();
    }

    if (!data->filters.empty()) {
        auto& filters_comp = entity.reassign<UglyFilter2D>();
        filters_comp.filters = data->filters;
//        ecs::replace_or_assign<node_filters_t>(entity);
    }
}

ecs::EntityApi create_and_merge(const SGFile& sg, SGFileRes asset,
                                const SGNodeData* data,
                                const SGNodeData* over = nullptr) {
    auto entity = ecs::create<Node, Transform2D, WorldTransform2D>();
    if (data) {
        apply(entity, data, asset);
    }
    if (over) {
        apply(entity, over, asset);
    }
    if (data) {
        for (const auto& child: data->children) {
            auto child_entity = create_and_merge(sg, asset, sg_get(sg, child.libraryName), &child);
            appendStrict(entity, child_entity);
        }
    }

    return entity;
}

void extend_bounds(const SGFile& file, const SGNodeData& data, bounds_builder_2f& boundsBuilder,
                   const matrix_2d& matrix) {
    const Res<Sprite> spr{data.sprite};
    if (spr) {
        boundsBuilder.add(spr->rect, matrix);
    }
    for (const auto& child: data.children) {
        const auto& symbol = child.libraryName.empty() ? child : *sg_get(file, child.libraryName);
        extend_bounds(file, symbol, boundsBuilder, matrix * child.matrix);
    }
}

ecs::EntityApi sg_create(const std::string& library, const std::string& name, ecs::EntityApi parent) {
    ecs::EntityApi result = nullptr;
    SGFileRes file{library};
    if (file) {
        const SGNodeData* data = sg_get(*file, name);
        if (data) {
            result = create_and_merge(*file, file, data);
            if (result && parent) {
                appendStrict(parent, result);
            }
        } else {
            EK_WARN_F("SG Object %s not found in library %s", name.c_str(), library.c_str());
        }
    } else {
        EK_WARN_F("SG not found: %s", library.c_str());
    }
    return result;
}

rect_f sg_get_bounds(const std::string& library, const std::string& name) {
    SGFileRes file{library};
    if (file) {
        const SGNodeData* data = sg_get(*file, name);
        if (data) {
            bounds_builder_2f bb{};
            extend_bounds(*file, *data, bb, data->matrix);
            return bb.rect();
        }
    }
    return {};
}

}