#include "Scanner.hpp"
#include "ShapeDecoder.hpp"

#include <ek/xfl/Doc.hpp>

namespace ek::xfl {

using Op = RenderCommand::Operation;

Scanner::Scanner() {
    reset();
}

void Scanner::reset() {
    stack_.clear();
    stack_.emplace_back();
    batches.clear();
    bounds = {};
}

void Scanner::drawInstance(const Doc& doc, const Element& element) {
    auto* s = doc.find(element.libraryItemName, element.elementType);
    if (s) {
        pushTransform(element);
        draw(doc, *s);
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
    const auto& end = layers.rend();
    for (auto it = layers.rbegin(); it != end; ++it) {
        const auto& layer = *it;
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

rect_f Scanner::getBounds(const Doc& doc, const std::vector<Element>& elements) {
    Scanner scanner{};
    for (auto& el: elements) {
        scanner.draw(doc, el);
    }
    return scanner.bounds.rect();
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

rect_f transform(const matrix_2d& m, const rect_f& rc) {
    const float2 corners[4] = {
            m.transform(rc.x, rc.y),
            m.transform(rc.right(), rc.y),
            m.transform(rc.right(), rc.bottom()),
            m.transform(rc.x, rc.bottom())
    };
    float xMin = corners[0].x;
    float yMin = corners[0].y;
    float xMax = corners[0].x;
    float yMax = corners[0].y;
    for (int i = 1; i < 4; ++i) {
        xMin = fmin(xMin, corners[i].x);
        yMin = fmin(yMin, corners[i].y);
        xMax = fmax(xMax, corners[i].x);
        yMax = fmax(yMax, corners[i].y);
    }
    return {xMin, yMin, xMax - xMin, yMax - yMin};
}

bool Scanner::render(const BitmapData* bitmap, const TransformModel& world) {
    RenderCommandsBatch batch{};
    batch.transform = world;
    RenderCommand cmd{Op::bitmap};
    cmd.bitmap = bitmap;
    batch.commands.push_back(cmd);

    const rect_f rc{0.0f, 0.0f,
                    static_cast<float>(bitmap->width),
                    static_cast<float>(bitmap->height)};
    batch.bounds.add(rc, world.matrix);

    return render(batch);
}


bool Scanner::render(const RenderCommandsBatch& batch) {
    if (!batch.commands.empty() && !batch.bounds.empty()) {
        batches.push_back(batch);
        bounds.add(batch.bounds.rect());
        return true;
    }
    return false;
}

bool Scanner::renderShapeObject(const Element& el, const TransformModel& world) {
    if (!el.shape) {
        return false;
    }
    const auto& shape = *el.shape;
    rect_f rc{shape.x, shape.y, shape.objectWidth, shape.objectHeight};
    Op op = el.elementType == ElementType::object_rectangle ? Op::rectangle : Op::oval;

    RenderCommand cmd{op};
    cmd.v[0] = rc.x;
    cmd.v[1] = rc.y;
    cmd.v[2] = rc.right();
    cmd.v[3] = rc.bottom();
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
    batch.bounds.add(rc, world.matrix, hw);
    return render(batch);
}


}