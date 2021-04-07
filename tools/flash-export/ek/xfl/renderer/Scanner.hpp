#pragma once

#include <ek/ds/Array.hpp>
#include <string>
#include <ek/math/bounds_builder.hpp>
#include <ek/xfl/types.hpp>
#include <ek/xfl/renderer/RenderCommand.hpp>

namespace ek::xfl {

class Doc;

struct Element;

class Scanner {
public:
    Scanner();

    void reset();

    void draw(const Doc& doc, const Element& element);

    void drawGroup(const Doc& doc, const Element& element);

    void drawShape(const Element& element);

    void drawInstance(const Doc& doc, const Element& element);

    void drawSymbolItem(const Doc& doc, const Element& element);

    static rect_f getBounds(const Doc& doc, const Array<Element>& elements);

/** Covert concrete objects to render commands **/

    bool render(const Element& el, const TransformModel& world);

    bool render(const RenderCommandsBatch& batch);

    bool render(const BitmapData* bitmap, const TransformModel& world);

    bool renderShapeObject(const Element& el, const TransformModel& world);

public:
    Array<RenderCommandsBatch> batches;
    bounds_builder_2f bounds{};

private:
    Array<TransformModel> stack_;

    void pushTransform(const Element& el);

    void popTransform();
};

}
