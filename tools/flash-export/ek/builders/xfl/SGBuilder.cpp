#include "SGBuilder.hpp"

#include "AnimationHelpers.hpp"
#include "RenderElement.hpp"

#include <ek/xfl/Doc.hpp>
#include <ek/xfl/renderer/Scanner.hpp>
#include <ek/util/StringUtil.hpp>
#include <ek/debug.hpp>

namespace ek::xfl {

using std::string;

enum BoundsMode {
    Bounds_None = 0,
    Bounds_Bounds = 1,
    Bounds_HitArea = 2,
    Bounds_Scissors = 4
};

int getBoundingRectFlags(const string& str) {
    int flags = 0;
    if (equals_ignore_case(str, "hitrect")) {
        flags |= Bounds_HitArea;
    }
    if (equals_ignore_case(str, "bbrect")) {
        flags |= Bounds_Bounds;
    }
    if (equals_ignore_case(str, "cliprect")) {
        flags |= Bounds_Scissors;
    }
    return flags;
}

bool setupSpecialLayer(const Doc& doc, const Layer& layer, ExportItem& toItem) {
    auto flags = getBoundingRectFlags(layer.name);
    if (flags != 0) {
        toItem.node.boundingRect = Scanner::getBounds(doc, layer.frames[0].elements);
        if ((flags & Bounds_HitArea) != 0) {
            toItem.node.hitAreaEnabled = true;
        }
        if ((flags & Bounds_Bounds) != 0) {
            toItem.node.boundsEnabled = true;
        }
        if ((flags & Bounds_Scissors) != 0) {
            toItem.node.scissorsEnabled = true;
        }
        return true;
    }
    return false;
}

void collectFramesMetaInfo(const Doc& doc, ExportItem& item) {
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

bool shouldConvertItemToSprite(ExportItem& item) {
    if (item.children.size() == 1 && item.drawingLayerChild) {
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

void process_transform(const Element& el, ExportItem& item) {
    item.node.matrix = el.transform.matrix;
    item.node.color = el.transform.color;
    item.node.visible = el.isVisible;
}

void process_filters(const Element& el, ExportItem& item) {
    for (auto& filter : el.filters) {
        SGFilter fd;
        fd.type = SGFilterType::None;
        fd.color = argb32_t{filter.color};
        fd.blur = filter.blur;
        fd.quality = filter.quality;

        float a = math::to_radians(filter.angle);
        fd.offset = filter.distance * float2{cosf(a), sinf(a)};

        if (filter.type == FilterType::drop_shadow) {
            fd.type = SGFilterType::DropShadow;
        } else if (filter.type == FilterType::glow) {
            fd.type = SGFilterType::Glow;
        }

        if (fd.type != SGFilterType::None) {
            item.node.filters.push_back(fd);
        }
    }
}

void processTextField(const Element& el, ExportItem& item, const Doc& doc) {
    auto& tf = item.node.dynamicText.emplace();
    //if(dynamicText.rect != null) {
//    item->node.matrix.tx += el.rect.x - 2;
//    item->node.matrix.ty += el.rect.y - 2;
    //}
    const auto& textRun = el.textRuns[0];
    string faceName = textRun.attributes.face;
    if (!faceName.empty() && faceName.back() == '*') {
        faceName.pop_back();
        const auto* fontItem = doc.find(faceName, ElementType::font_item, true);
        if (fontItem) {
            faceName = fontItem->font;
        }
    }
    // animate exports as CR line-ending (legacy MacOS),
    // we need only LF to not check twice when drawing the text
    tf.text = replace(textRun.characters, '\r', '\n');
    tf.font = faceName;
    tf.size = textRun.attributes.size;
    if (el.lineType == TextLineType::Multiline) {
        tf.wordWrap = true;
    }
    tf.rect = expand(el.rect, 2.0f);
//    tf.rect = el.rect;
    tf.alignment = textRun.attributes.alignment;
    tf.lineHeight = textRun.attributes.lineHeight;
    tf.lineSpacing = textRun.attributes.lineSpacing;

    SGTextLayerData layer;
    layer.color = argb32_t{textRun.attributes.color};
    tf.layers.push_back(layer);

    for (auto& filter : el.filters) {
        layer.color = argb32_t{filter.color};
        layer.blurRadius = std::fmin(filter.blur.x, filter.blur.y);
        layer.blurIterations = filter.quality;
        layer.offset = {};
        if (filter.type == FilterType::drop_shadow) {
            const float a = math::to_radians(filter.angle);
            layer.offset = filter.distance * float2{cosf(a), sinf(a)};
        }
        layer.strength = int(filter.strength);
        tf.layers.push_back(layer);
    }
}

SGBuilder::SGBuilder(const Doc& doc)
        : doc{doc} {
}

SGBuilder::~SGBuilder() = default;

void SGBuilder::build_library() {

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

    Array<ExportItem*> chi{};
    for (auto& item : library.children) {
        if (item->usage > 0) {
            chi.push_back(item);
        } else {
            delete item;
        }
    }
    library.children = chi;
}

SGFile SGBuilder::export_library() {

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

    SGFile sg;
    sg.linkages = linkages;
    for (auto& pair : doc.scenes) {
        sg.scenes.push_back(pair.second);
    }

    for (auto& item : library.node.children) {
        if (item.sprite == item.libraryName
            && item.scaleGrid.empty()
            && !isInLinkages(item.libraryName)) {
            continue;
        }
        sg.library.push_back(item);
    }

    return sg;
}

void SGBuilder::process_symbol_instance(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    assert(el.elementType == ElementType::symbol_instance);

    auto* item = new ExportItem();
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = el.item.name;
    item->node.libraryName = el.libraryItemName;
    item->node.button = el.symbolType == SymbolType::button;
    item->node.touchable = !el.silent;

    process_filters(el, *item);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_bitmap_instance(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    assert(el.elementType == ElementType::bitmap_instance);

    auto* item = new ExportItem;
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

void SGBuilder::process_bitmap_item(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    auto* item = new ExportItem();
    item->ref = &el;
    item->node.libraryName = el.item.name;
    item->renderThis = true;
    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_dynamic_text(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    assert(el.elementType == ElementType::dynamic_text);

    auto* item = new ExportItem();
    item->ref = &el;
    process_transform(el, *item);
    item->node.name = el.item.name;

    processTextField(el, *item, doc);

    item->append_to(parent);
    if (bag) {
        bag->list.push_back(item);
    }
}

void SGBuilder::process_symbol_item(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    assert(el.elementType == ElementType::symbol_item ||
           el.elementType == ElementType::scene_timeline);

    auto* item = new ExportItem();
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
            const auto& layers = el.timeline.layers;
            for (int layerIndex = int(layers.size()) - 1; layerIndex >= 0; --layerIndex) {
                const auto& layer = layers[layerIndex];
                if (layer.layerType == LayerType::normal) {
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

void SGBuilder::process_group(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    assert(el.elementType == ElementType::group);
    for (const auto& member : el.members) {
        process(member, parent, bag);
    }
}

void SGBuilder::process_shape(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    assert(el.elementType == ElementType::shape ||
           el.elementType == ElementType::object_oval ||
           el.elementType == ElementType::object_rectangle);
    auto* item = addElementToDrawingLayer(parent, el);
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

ExportItem* SGBuilder::addElementToDrawingLayer(ExportItem* item, const Element& el) {
    if (item->drawingLayerChild) {
        auto* child = item->drawingLayerChild;
        if (item->children.back() == child &&
            child->drawingLayerItem &&
            child->animationSpan0 == _animationSpan0 &&
            child->animationSpan1 == _animationSpan1) {
            // EK_DEBUG << "Found drawing layer " << child->ref->item.name;
            auto& timeline = child->drawingLayerItem->timeline;
            assert(!timeline.layers.empty());
            assert(!timeline.layers[0].frames.empty());
            timeline.layers[0].frames[0].elements.push_back(el);
            child->shapes++;
            return child;
        }
    }
    auto shapeItem = std::make_unique<Element>();
    const std::string name = SHAPE_ID + std::to_string(++NEXT_SHAPE_IDX);
    {
        shapeItem->item.name = name;
        shapeItem->elementType = ElementType::symbol_item;
        auto& layer = shapeItem->timeline.layers.emplace_back();
        auto& frame = layer.frames.emplace_back();
        frame.elements.push_back(el);
    }

    auto* layer = new ExportItem();
    layer->ref = shapeItem.get();
    layer->node.libraryName = name;
    layer->renderThis = true;
    layer->animationSpan0 = _animationSpan0;
    layer->animationSpan1 = _animationSpan1;
    layer->append_to(&library);

    auto shapeInstance = std::make_unique<Element>();
    shapeInstance->libraryItemName = name;
    shapeInstance->elementType = ElementType::symbol_instance;

    processing_bag_t bag;
    process(*shapeInstance, item, &bag);
    auto* drawingLayerInstance = bag.list[0];
    drawingLayerInstance->drawingLayerInstance = std::move(shapeInstance);
    drawingLayerInstance->drawingLayerItem = std::move(shapeItem);
    item->drawingLayerChild = drawingLayerInstance;
    item->shapes++;
    // EK_DEBUG << "Created drawing layer " << newElement->item.name;
    return drawingLayerInstance;
}

void SGBuilder::process(const Element& el, ExportItem* parent, processing_bag_t* bag) {
    const auto type = el.elementType;
    switch (type) {
        case ElementType::symbol_instance:
            process_symbol_instance(el, parent, bag);
            break;
        case ElementType::bitmap_instance:
            process_bitmap_instance(el, parent, bag);
            break;
        case ElementType::bitmap_item:
            process_bitmap_item(el, parent, bag);
            break;
        case ElementType::symbol_item:
        case ElementType::scene_timeline:
            process_symbol_item(el, parent, bag);
            break;
        case ElementType::dynamic_text:
            process_dynamic_text(el, parent, bag);
            break;
        case ElementType::group:
            process_group(el, parent, bag);
            break;
        case ElementType::shape:
        case ElementType::object_oval:
        case ElementType::object_rectangle:
            process_shape(el, parent, bag);
            break;

        case ElementType::font_item:
        case ElementType::sound_item:
        case ElementType::static_text:
            EK_WARN << "element type is not supported yet:" << static_cast<int>(type);
            break;

        case ElementType::unknown:
            EK_WARN << "unknown element type:" << static_cast<int>(type);
            break;
    }
}

/*** rendering ***/

void SGBuilder::render(const ExportItem& item, MultiResAtlasData& toAtlas) const {
    const Element& el = *item.ref;
    const auto spriteID = el.item.name;
    RenderElementOptions options;
    for (auto& resolution : toAtlas.resolutions) {
        options.scale = std::min(
                item.max_abs_scale,
                resolution.resolution_scale * std::min(1.0f, item.estimated_scale)
        );
        auto res = renderElement(doc, el, options);
        res.name = spriteID;
        res.trim = item.node.scaleGrid.empty();
        resolution.sprites.push_back(res);
    }
}

void SGBuilder::build_sprites(MultiResAtlasData& to_atlas) const {
    for (auto* item : library.children) {
        if (item->renderThis) {
            item->node.sprite = item->ref->item.name;
            render(*item, to_atlas);
        }
        if (!item->node.scaleGrid.empty()) {

        }
    }
}

bool SGBuilder::isInLinkages(const string& id) const {
    for (const auto& pair : linkages) {
        if (pair.second == id) {
            return true;
        }
    }
    return false;
}

SGMovieLayerData* findTargetLayer(SGMovieData& movie, const SGNodeData* item) {
    for (auto& layer : movie.layers) {
        for (const auto* t : layer.targets) {
            if (t == item) {
                return &layer;
            }
        }
    }
    return nullptr;
}

void SGBuilder::processTimeline(const Element& el, ExportItem* item) {
    auto& movie = item->node.movie.emplace();
    movie.frames = el.timeline.getTotalFrames();
    movie.fps = doc.info.frameRate;

    const auto& layers = el.timeline.layers;
    for (int layerIndex = int(layers.size()) - 1; layerIndex >= 0; --layerIndex) {
        auto& layer = layers[layerIndex];
        // ignore other layers.
        // TODO: mask layer
        if (layer.layerType != LayerType::normal) {
            continue;
        }
        const int framesTotal = static_cast<int>(layer.frames.size());
        for (int frameIndex = 0; frameIndex < framesTotal; ++frameIndex) {
            auto& frame = layer.frames[frameIndex];
            processing_bag_t targets;
            for (const auto& frameElement : frame.elements) {
                bool found = false;
                for (auto* prevItem : item->children) {
                    if (prevItem->ref &&
                        prevItem->ref->libraryItemName == frameElement.libraryItemName &&
                        prevItem->ref->item.name == frameElement.item.name &&
                        prevItem->fromLayer == layerIndex &&
                        prevItem->movieLayerIsLinked) {
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
            std::optional<SGKeyFrameTransform> delta;
            if (k0.motion_type == 1
                && !frame.elements.empty()
                && (frameIndex + 1) < framesTotal) {
                const auto& nextFrame = layer.frames[frameIndex + 1];
                if (!nextFrame.elements.empty()) {
                    const auto& el0 = frame.elements.back();
                    const auto& el1 = nextFrame.elements[0];
                    delta.emplace(extractTweenDelta(frame, el0, el1));
                }
            }
            for (auto* target : targets.list) {
                if (target->ref) {
                    auto* targetNodeRef = &target->node;
                    SGMovieLayerData* targetLayer = nullptr;
                    if (!target->movieLayerIsLinked) {
                        targetLayer = &movie.layers.emplace_back();
                        targetLayer->targets.push_back(targetNodeRef);
                        target->fromLayer = layerIndex;
                        target->movieLayerIsLinked = true;
                    } else {
                        targetLayer = findTargetLayer(movie, targetNodeRef);
                        assert(targetLayer);
                    }

                    auto kf0 = createFrameModel(frame);
                    setupFrameFromElement(kf0, *target->ref);
                    targetLayer->frames.push_back(kf0);
                    if (delta) {
                        SGMovieFrameData kf1{};
                        kf1.index = kf0.index + kf0.duration;
                        kf1.duration = 0;
                        kf1.transform = kf0.transform + *delta;
                        kf1.visible = false;
                        targetLayer->frames.push_back(kf1);
                    }
                }
            }
        }

        auto& movieLayers = movie.layers;
        for (auto it = movieLayers.begin(); it != movieLayers.end();) {
            bool empty = it->frames.empty();
            if (it->frames.size() == 1) {
                const auto& frame = it->frames[0];
                if (frame.index == 0 && frame.motion_type != 2 && frame.duration == movie.frames) {
                    empty = true;
                }
            }
            if (empty) {
                movieLayers.eraseIterator(it);
            } else {
                it++;
            }
        }

        if (movie.frames > 1 && !movieLayers.empty()) {
            for (size_t i = 0; i < movieLayers.size(); ++i) {
                for (auto* target : movieLayers[i].targets) {
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