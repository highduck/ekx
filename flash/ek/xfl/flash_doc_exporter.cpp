#include "flash_doc_exporter.hpp"
#include "animation_utility.hpp"

#include <ek/flash/doc/flash_doc.hpp>
#include <ek/flash/rasterizer/render_to_sprite.hpp>
#include <ek/flash/rasterizer/dom_scanner.hpp>
#include <ek/util/strings.hpp>
#include <ek/util/logger.hpp>

namespace ek::flash {

using std::string;


bool is_hit_rect(const string& str) {
    return equals_ignore_case(str, "hitrect");
}

bool is_clip_rect(const string& str) {
    return equals_ignore_case(str, "cliprect");
}

bool setupSpecialLayer(const flash_doc& doc, const layer_t& layer, export_item_t& toItem) {
    if (is_hit_rect(layer.name)) {
        toItem.node.hitRect = estimate_bounds(doc, layer.frames[0].elements);
        return true;
    } else if (is_clip_rect(layer.name)) {
        toItem.node.clipRect = estimate_bounds(doc, layer.frames[0].elements);
        return true;
    }
    return false;
}

void collectFramesMetaInfo(const flash_doc& doc, export_item_t& item) {
    if (!item.ref) {
        return;
    }
    auto& layers = item.ref->timeline.layers;
    for (auto& layer : layers) {
        if (setupSpecialLayer(doc, layer, item)) {
            continue;
        }
        for (auto& frame : layer.frames) {
            if (!frame.script.empty()) {
                item.node.scripts[frame.index] = frame.script;
            }
            if (!frame.name.empty()) {
                item.node.labels[frame.index] = frame.name;
            }
        }
    }
}

bool shouldConvertItemToSprite(export_item_t& item) {
    if (item.children.size() == 1 && item.drawingLayerChild && item.drawingLayerChild->shapes > 0) {
        return true;
    } else if (item.node.labels[0] == "*static") {
        // special user TAG
        return true;
    } else if (!item.node.scaleGrid.empty()) {
        // scale 9 grid items
        return true;
    }
    return false;
}

void process_transform(const element_t& el, export_item_t& item) {
    item.node.matrix = el.matrix;
    item.node.color = el.color;
    item.node.visible = el.isVisible;
}

void process_filters(const element_t& el, export_item_t& item) {
    for (auto& filter : el.filters) {
        float a = math::to_radians(filter.angle);
        float d = filter.distance;
        float2 offset{d * cosf(a), d * sinf(a)};

        filter_data fd;
        fd.type = sg_filter_type::none;
        fd.color = argb32_t{filter.color};
        fd.offset = offset;
        fd.blur = filter.blur;
        fd.quality = filter.quality;

        if (filter.type == filter_kind_t::drop_shadow) {
            fd.type = sg_filter_type::drop_shadow;
        } else if (filter.type == filter_kind_t::glow) {
            fd.type = sg_filter_type::glow;
        }

        if (fd.type != sg_filter_type::none) {
            item.node.filters.push_back(fd);
        }
    }
}

flash_doc_exporter::flash_doc_exporter(const flash_doc& doc)
        : doc{doc} {
}

flash_doc_exporter::~flash_doc_exporter() = default;

void flash_doc_exporter::build_library() {

    for (const auto& item: doc.library) {
        process(item, &library);
    }

    for (auto* item : library.children) {
        if (item->ref && item->ref->item.linkageExportForAS) {
            auto& linkageName = item->ref->item.linkageClassName;
            if (!linkageName.empty()) {
                linkages[linkageName] = item->ref->item.name;
            }
            item->inc_ref(library);
            item->update_scale(library, library.node.matrix);
        }
    }

    std::vector<export_item_t*> chi{};
    for (auto& item : library.children) {
        if (item->usage > 0) {
            chi.push_back(item);
        } else {
            // TODO:
//            delete item;
//            item = nullptr;
        }
    }
    library.children = chi;
}

sg_file flash_doc_exporter::export_library() {

    for (auto* item : library.children) {
        // CHANGE: we disable sprite assignment here
//        if (item->ref && (item->shapes > 0 || item->ref->bitmap)) {
//            item->node.sprite = item->node.libraryName;
//        }

        for (auto* child : item->children) {
            item->node.children.push_back(child->node);
        }

        // if item should be in global registry,
        // but if it's inline sprite - it's ok to throw it away
        if (!item->node.libraryName.empty()) {
            library.node.children.push_back(item->node);
        }
    }

    for (auto& item : library.node.children) {
        for (auto& child : item.children) {
            const auto* ref = library.find_library_item(child.libraryName);
            if (ref &&
                ref->node.sprite == ref->node.libraryName &&
                ref->node.scaleGrid.empty()
                    ) {
                child.sprite = ref->node.sprite;
                child.libraryName = "";
            }
        }
    }

    // for (const item of this.library.node.children) {
    //     if (item.children.length === 1) {
    //         const child = item.children[0];
    //         if(child.sprite && child.scaleGrid.empty) {
    //             item.sprite = child.sprite;
    //             item.children.length = 0;
    //         }
    //     }
    // }

    sg_file sg;
    sg.linkages = linkages;

    for (auto& item : library.node.children) {
        if (item.sprite == item.libraryName
            && item.scaleGrid.empty()
            && !isInLinkages(item.libraryName)) {
            continue;
        }
        sg.library.children.push_back(item);
    }

    return sg;
}

void flash_doc_exporter::process_symbol_instance(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    assert(el.elementType == element_type::symbol_instance);

    auto* item = new export_item_t();
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = el.item.name;
    item->node.libraryName = el.libraryItemName;
    item->node.button = el.symbolType == symbol_type::button;
    item->node.touchable = !el.silent;

    process_filters(el, *item);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void flash_doc_exporter::process_bitmap_instance(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    assert(el.elementType == element_type::bitmap_instance);

    auto* item = new export_item_t;
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = el.item.name;
    item->node.libraryName = el.libraryItemName;

    process_filters(el, *item);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void flash_doc_exporter::process_bitmap_item(const element_t& el, export_item_t* lib, processing_bag_t* bag) {
    auto* item = new export_item_t();
    item->ref = &el;
    item->node.libraryName = el.item.name;
    item->renderThis = true;
    item->append_to(lib);
    if (bag) {
        bag->list.push_back(item);
    }
}

void flash_doc_exporter::process_dynamic_text(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    assert(el.elementType == element_type::dynamic_text);

    auto* item = new export_item_t();
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = el.item.name;

    //if(dynamicText.rect != null) {
//    item->node.matrix.tx += el.rect.x - 2;
//    item->node.matrix.ty += el.rect.y - 2;
    //}
    string face = el.textRuns[0].attributes.face;
    if (!face.empty() && face.back() == '*') {
        face.pop_back();
        const auto* fontItem = doc.find(face, element_type::font_item);
        if (fontItem) {
            face = fontItem->font;
        }
    }

    item->node.dynamicText.emplace();
    item->node.dynamicText->rect = expand(el.rect, 2.0f);
    // TODO: replace '\r' to '\n' ?
    item->node.dynamicText->text = el.textRuns[0].characters;
    item->node.dynamicText->alignment = el.textRuns[0].attributes.alignment;
    item->node.dynamicText->face = face;
    item->node.dynamicText->size = el.textRuns[0].attributes.size;
    item->node.dynamicText->line_height = el.textRuns[0].attributes.line_height;
    item->node.dynamicText->line_spacing = el.textRuns[0].attributes.line_spacing;
    item->node.dynamicText->color = argb32_t{el.textRuns[0].attributes.color};

    process_filters(el, *item);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void flash_doc_exporter::process_symbol_item(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    assert(el.elementType == element_type::symbol_item ||
           el.elementType == element_type::scene_timeline);

    auto* item = new export_item_t();
    item->ref = &el;
    process_transform(el, *item);
    item->node.libraryName = el.item.name;
    assert(el.libraryItemName.empty());
    item->node.scaleGrid = el.scaleGrid;

    collectFramesMetaInfo(doc, *item);


    const auto frames_count = el.timeline.getTotalFrames();
    const auto elements_count = el.timeline.getElementsCount();

    if (shouldConvertItemToSprite(*item)) {
        item->renderThis = true;
        item->children.clear();
    } else {
        const auto withoutTimeline = frames_count <= 1 ||
                                     elements_count == 0 ||
                                     el.item.linkageBaseClass == "flash.display.Sprite" ||
                                     el.item.linkageBaseClass == "flash.display.Shape";

        if (withoutTimeline) {
            const auto layers = el.timeline.layers;
            for (int layerIndex = int(layers.size()) - 1; layerIndex >= 0; --layerIndex) {
                const auto& layer = layers[layerIndex];
                if (layer.layerType == layer_type::normal) {
                    for (auto& frame : layer.frames) {
                        for (auto& frameElement : frame.elements) {
                            _animationSpan0 = 0;
                            _animationSpan1 = 0;
                            process(frameElement, item);
                        }
                    }
                }
            }
            if (item->children.size() == 1 && item->drawingLayerChild) {
                item->renderThis = true;
                item->children.clear();
            }
        } else {
            processTimeline(el, item);
        }
    }

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void flash_doc_exporter::process_group(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    assert(el.elementType == element_type::group);
    for (const auto& member : el.members) {
        process(member, parent, bag);
    }
}

void flash_doc_exporter::process_shape(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    assert(el.elementType == element_type::shape ||
           el.elementType == element_type::object_oval ||
           el.elementType == element_type::object_rectangle);
    const auto item = addElementToDrawingLayer(parent, el);
    if (bag) {
        bag->list.push_back(item);
    }
//    if (parent) {
//        parent->shapes++;
//    }
}

static std::string SHAPE_ID = "$";
// we need global across all libraries to avoid multiple FLA exports overlapping
int NEXT_SHAPE_IDX = 0;

export_item_t* flash_doc_exporter::addElementToDrawingLayer(export_item_t* item, const element_t& el) {
    if (item->drawingLayerChild) {
        auto* child = item->drawingLayerChild;
        if (child->drawingLayerElement && child->ref &&
            child->animationSpan0 == _animationSpan0 &&
            child->animationSpan1 == _animationSpan1) {
            EK_DEBUG << "Found drawing layer " << child->ref->item.name;
            child->drawingLayerElement->members.push_back(el);
            child->shapes++;
            return child;
        }
    }
    auto* layer = new export_item_t();
    auto* newElement = new element_t();
    const std::string name = SHAPE_ID + std::to_string(++NEXT_SHAPE_IDX);
    newElement->libraryItemName = name;
    newElement->item.name = name;
    newElement->elementType = element_type::group;
    newElement->members.push_back(el);
    layer->ref = newElement;
    layer->node.libraryName = name;
    layer->renderThis = true;
    layer->drawingLayerElement = newElement;
    layer->animationSpan0 = _animationSpan0;
    layer->animationSpan1 = _animationSpan1;
    item->drawingLayerChild = layer;
    item->add(layer);
    library.add(layer);

    EK_DEBUG << "Created drawing layer " << newElement->item.name;
    layer->shapes++;
    return layer;
}

void flash_doc_exporter::process(const element_t& el, export_item_t* parent, processing_bag_t* bag) {
    const auto type = el.elementType;
    switch (type) {
        case element_type::symbol_instance:
            process_symbol_instance(el, parent, bag);
            break;
        case element_type::bitmap_instance:
            process_bitmap_instance(el, parent, bag);
            break;
        case element_type::bitmap_item:
            process_bitmap_item(el, parent, bag);
            break;
        case element_type::symbol_item:
        case element_type::scene_timeline:
            process_symbol_item(el, parent, bag);
            break;
        case element_type::dynamic_text:
            process_dynamic_text(el, parent, bag);
            break;
        case element_type::group:
            process_group(el, parent, bag);
            break;
        case element_type::shape:
        case element_type::object_oval:
        case element_type::object_rectangle:
            process_shape(el, parent, bag);
            break;

        case element_type::font_item:
        case element_type::sound_item:
        case element_type::static_text:
            EK_WARN << "element type is not supported yet:" << static_cast<int>(type);
            break;

        case element_type::unknown:
            EK_WARN << "unknown element type:" << static_cast<int>(type);
            break;
    }
}

/*** rendering ***/

void flash_doc_exporter::render(const export_item_t& item, spritepack::atlas_t& to_atlas) const {
    const element_t& el = *item.ref;
    const auto spriteID = el.item.name;
    renderer_options_t options;
    for (auto& resolution : to_atlas.resolutions) {
        options.scale = std::min(
                item.max_abs_scale,
                resolution.resolution_scale * std::min(1.0f, item.estimated_scale)
        );
        auto res = ::ek::flash::render(doc, el, options);
        res.name = spriteID;
        res.trim = item.node.scaleGrid.empty();
        resolution.sprites.push_back(res);
    }
}

void flash_doc_exporter::build_sprites(spritepack::atlas_t& to_atlas) const {
    for (auto* item : library.children) {
        if (item->renderThis) {
            item->node.sprite = item->ref->item.name;
            render(*item, to_atlas);
        }
        if (!item->node.scaleGrid.empty()) {

        }
    }
}

bool flash_doc_exporter::isInLinkages(const string& id) const {
    for (const auto& pair : linkages) {
        if (pair.second == id) {
            return true;
        }
    }
    return false;
}

void flash_doc_exporter::processTimeline(const element_t& el, export_item_t* item) {
    auto& movie = item->node.movie.emplace();
    movie.frames = el.timeline.getTotalFrames();
    movie.fps = doc.info.frameRate;

    const auto& layers = el.timeline.layers;
    for (int layerIndex = int(layers.size()) - 1; layerIndex >= 0; --layerIndex) {
        auto& layer = layers[layerIndex];
        // ignore other layers.
        // TODO: mask layer
        if (layer.layerType != layer_type::normal) {
            continue;
        }
        for (int frameIndex = 0; frameIndex < layer.frames.size(); ++frameIndex) {
            auto& frame = layer.frames[frameIndex];
            processing_bag_t targets;
            for (const auto& frameElement : frame.elements) {
                bool found = false;
                for (auto* prevItem : item->children) {
                    if (prevItem->ref &&
                        prevItem->ref->libraryItemName == frameElement.libraryItemName &&
                        prevItem->ref->item.name == frameElement.item.name &&
                        prevItem->fromLayer == layerIndex &&
                        prevItem->linkedMovieLayer) {
                        targets.list.push_back(prevItem);
                        // copy new transform
                        prevItem->ref = &frameElement;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    _animationSpan0 = frame.index;
                    _animationSpan1 = frame.endFrame();
                    process(frameElement, item, &targets);
                }
            }
            const auto k0 = createFrameModel(frame);
            std::optional<keyframe_transform_t> delta;
            if (k0.motion_type == 1
                && !frame.elements.empty()
                && (frameIndex + 1) < layer.frames.size()) {
                const auto& nextFrame = layer.frames[frameIndex + 1];
                if (!nextFrame.elements.empty()) {
                    const auto& el0 = frame.elements.back();
                    const auto& el1 = nextFrame.elements[0];
                    delta.emplace(extractTweenDelta(frame, el0, el1));
                }
            }
            for (auto* target : targets.list) {
                if (target->ref) {

                    if (!target->linkedMovieLayer) {
                        auto& targetLayer_ = movie.layers.emplace_back();
                        targetLayer_.targets.push_back(&target->node);
                        target->fromLayer = layerIndex;
                        target->linkedMovieLayer = &targetLayer_;
                    }
                    auto* targetLayer = target->linkedMovieLayer;

                    auto kf0 = createFrameModel(frame);
                    setupFrameFromElement(kf0, *target->ref);
                    targetLayer->frames.push_back(kf0);
                    if (delta) {
                        movie_frame_data kf1{};
                        kf1.index = kf0.index + kf0.duration;
                        kf1.duration = 0;
                        kf1.position = kf0.position + delta->position;
                        kf1.pivot = kf0.pivot + delta->pivot;
                        kf1.scale = kf0.scale + delta->scale;
                        kf1.skew = kf0.skew + delta->skew;
                        kf1.color.multiplier = kf0.color.multiplier + delta->color.multiplier;
                        kf1.color.offset = kf0.color.offset + delta->color.offset;
                        kf1.visible = false;
                        targetLayer->frames.push_back(kf1);
                    }
                }
            }
        }

        auto it = movie.layers.begin();
        const auto end = movie.layers.end();
        while (it != movie.layers.end()) {
            bool empty = false;
            if (it->frames.empty()) {
                empty = true;
            }
            if (it->frames.size() == 1) {
                const auto& frame = it->frames[0];
                if (frame.index == 0 && frame.motion_type != 2 && frame.duration == movie.frames) {
                    empty = true;
                }
            }
            if (empty) {
                it = movie.layers.erase(it);
            } else {
                it++;
            }
        }

        if (movie.frames > 1 && !movie.layers.empty()) {
            for (size_t i = 0; i < movie.layers.size(); ++i) {
                for (auto* target : movie.layers[i].targets) {
                    target->movieTargetId = static_cast<int>(i);
                }
            }
            item->node.movie = movie;
        }

        _animationSpan0 = 0;
        _animationSpan1 = 0;
    }
}

}