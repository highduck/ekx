#include "Scanner.hpp"
#include "ShapeDecoder.hpp"

#include "../Doc.hpp"

namespace ek::xfl {

using Op = RenderCommand::Operation;

Scanner::Scanner() {
    reset();
}

void Scanner::reset() {
    stack_.clear();
    stack_.emplace_back();
    batches.clear();
    bounds = brect_inf();
}

void Scanner::drawInstance(const Doc& doc, const Element& element) {
    auto itemType = ElementType::symbol_item;
    if (element.elementType == ElementType::bitmap_instance) {
        itemType = ElementType::bitmap_item;
    }
    auto* item = doc.find(element.libraryItemName, itemType);
    if (item) {
        pushTransform(element);
        draw(doc, *item);
        popTransform();
    }
}

void Scanner::drawGroup(const Doc& doc, const Element& element) {
    // ! Group Transformation is not applied !
    for (const auto& member : element.members) {
        draw(doc, member);
    }
}

void Scanner::drawShape(const Element& element) {
    render(element, stack_.back() * element.transform);
}

void Scanner::drawSymbolItem(const Doc& doc, const Element& element) {
    const auto& layers = element.timeline.layers;

    // go backward
    const uint32_t count = layers.size();
    for(uint32_t i = count - 1; i < count; --i) {
        const auto& layer = layers[i];
        if (layer.layerType == LayerType::normal) {
            if (!layer.frames.empty()) {
                for (const auto& el : layer.frames[0].elements) {
                    draw(doc, el);
                }
            }
        }
    }
}

void Scanner::draw(const Doc& doc, const Element& element) {
    switch (element.elementType) {
        case ElementType::symbol_instance:
        case ElementType::bitmap_instance:
            drawInstance(doc, element);
            break;
        case ElementType::symbol_item:
        case ElementType::scene_timeline:
            drawSymbolItem(doc, element);
            break;
        case ElementType::group:
            drawGroup(doc, element);
            break;
        case ElementType::shape:
        case ElementType::object_rectangle:
        case ElementType::object_oval:
        case ElementType::bitmap_item:
            drawShape(element);
            break;
        default:
            break;
    }
}

void Scanner::pushTransform(const Element& el) {
    stack_.push_back(stack_.back() * el.transform);
}

void Scanner::popTransform() {
    stack_.pop_back();
}

rect_t Scanner::getBounds(const Doc& doc, const Array<Element>& elements) {
    Scanner scanner{};
    for (auto& el: elements) {
        scanner.draw(doc, el);
    }
    return brect_get_rect(scanner.bounds);
}

// Convert concrete objects to render commands

bool Scanner::render(const Element& el, const TransformModel& world) {
    switch (el.elementType) {
        case ElementType::bitmap_item:
            return render(el.bitmap.get(), world);
        case ElementType::object_oval:
        case ElementType::object_rectangle:
            return renderShapeObject(el, world);
        default:
            break;
    }
    ShapeDecoder decoder{world};
    decoder.decode(el);
    return !decoder.empty() && render(decoder.result());
}

rect_t transform(const mat3x2_t m, const rect_t rc) {
    return rect_transform(rc, m);
}

bool Scanner::render(const BitmapData* bitmap, const TransformModel& world) {
    RenderCommandsBatch batch{};
    batch.transform = world;
    RenderCommand cmd{Op::bitmap};
    cmd.bitmap = bitmap;
    batch.commands.push_back(cmd);

    const rect_t rc = rect_wh((float)bitmap->width, (float)bitmap->height);
    batch.bounds = brect_extend_transformed_rect(batch.bounds, rc, world.matrix);

    return render(batch);
}


bool Scanner::render(const RenderCommandsBatch& batch) {
    if (!batch.commands.empty() && !brect_is_empty(batch.bounds)) {
        batches.push_back(batch);
        bounds = brect_extend_rect(bounds, brect_get_rect(batch.bounds));
        return true;
    }
    return false;
}

bool Scanner::renderShapeObject(const Element& el, const TransformModel& world) {
    if (!el.shape) {
        return false;
    }
    const auto& shape = *el.shape;
    rect_t rc = rect(shape.x, shape.y, shape.objectWidth, shape.objectHeight);
    Op op = el.elementType == ElementType::object_rectangle ? Op::rectangle : Op::oval;

    RenderCommand cmd{op};
    cmd.v[0] = rc.x;
    cmd.v[1] = rc.y;
    cmd.v[2] = RECT_R(rc);
    cmd.v[3] = RECT_B(rc);
    switch (el.elementType) {
        case ElementType::object_rectangle:
            cmd.v[4] = shape.topLeftRadius;
            cmd.v[5] = shape.topRightRadius;
            cmd.v[6] = shape.bottomRightRadius;
            cmd.v[7] = shape.bottomLeftRadius;
            break;
        case ElementType::object_oval:
            cmd.v[4] = shape.startAngle;
            cmd.v[5] = shape.endAngle;
            cmd.v[6] = shape.closePath ? 1 : 0;
            cmd.v[7] = shape.innerRadius / 100.0;
            break;
        default:
            return false;
    }

    cmd.fill = !el.fills.empty() ? &el.fills[0] : nullptr;
    cmd.stroke = !el.strokes.empty() > 0 ? &el.strokes[0] : nullptr;

    RenderCommandsBatch batch{};
    batch.transform = world;
    batch.commands.push_back(cmd);

    const float hw = cmd.stroke ? (cmd.stroke->weight / 2.0f) : 0.0f;
    batch.bounds = brect_extend_transformed_rect(batch.bounds, rect_expand(rc, hw), world.matrix);
    return render(batch);
}


}