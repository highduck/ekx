#pragma once

#include "ExportItem.hpp"
#include <ek/util/NoCopyAssign.hpp>
#include "../ImageSet.hpp"
#include <unordered_map>

namespace ek::xfl {

class Doc;

class SGBuilder : private NoCopyAssign {
public:
    const Doc& doc;
    ExportItem library;
    std::unordered_map<std::string, std::string> linkages;

private:
    int _animationSpan0 = 0;
    int _animationSpan1 = 0;

public:
    explicit SGBuilder(const Doc& doc);

    ~SGBuilder();

    void build_library();

    void build_sprites(ImageSet& toImageSet) const;

    void process(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_symbol_instance(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_symbol_item(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_bitmap_instance(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_bitmap_item(const Element& el, ExportItem* library, processing_bag_t* bag = nullptr);

    void process_dynamic_text(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_group(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void process_shape(const Element& el, ExportItem* parent, processing_bag_t* bag = nullptr);

    void render(const ExportItem& item, ImageSet& toImageSet) const;

    [[nodiscard]]
    SGFile export_library();

    ExportItem* addElementToDrawingLayer(ExportItem* item, const Element& el);

private:
    [[nodiscard]] bool isInLinkages(const std::string& id) const;

    void processTimeline(const Element& Element, ExportItem* PItem);
};

}