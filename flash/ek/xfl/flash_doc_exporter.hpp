#pragma once

#include "export_item.hpp"
#include <ek/spritepack/sprite_data.hpp>
#include <ek/scenex/data/sg_data.hpp>
#include <unordered_map>

namespace ek::flash {

class flash_doc;

struct processing_bag_t {
    std::vector<export_item_t*> list;
};

class flash_doc_exporter : private disable_copy_assign_t {
public:
    const flash_doc& doc;
    export_item_t library;
    std::unordered_map<std::string, std::string> linkages;

private:
    int _animationSpan0 = 0;
    int _animationSpan1 = 0;

public:
    explicit flash_doc_exporter(const flash_doc& doc);

    ~flash_doc_exporter();

    void build_library();

    void build_sprites(spritepack::atlas_t& to_atlas) const;

    void process(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void process_symbol_instance(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void process_symbol_item(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void process_bitmap_instance(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void process_bitmap_item(const element_t& el, export_item_t* library, processing_bag_t* bag = nullptr);

    void process_dynamic_text(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void process_group(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void process_shape(const element_t& el, export_item_t* parent, processing_bag_t* bag = nullptr);

    void render(const export_item_t& item, spritepack::atlas_t& to_atlas) const;

    [[nodiscard]]
    sg_file export_library();

    export_item_t* addElementToDrawingLayer(export_item_t* item, const element_t& el);

private:
    [[nodiscard]] bool isInLinkages(const std::string& id) const;

    void processTimeline(const element_t& Element, export_item_t* PItem);
};

}