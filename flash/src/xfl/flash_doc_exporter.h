#pragma once

#include "export_item.h"
#include <ek/spritepack/sprite_data.hpp>
#include <ek/scenex/data/sg_data.h>
#include <unordered_map>

namespace ek::flash {

class flash_file;

class flash_doc_exporter {
public:
    const flash_file& doc;
    export_item_t library;
    std::unordered_map<std::string, std::string> linkages;

    explicit flash_doc_exporter(const flash_file& doc);

    ~flash_doc_exporter();

    void build_library();

    void build_sprites(spritepack::atlas_t& to_atlas) const;

    void process_element(const element_t& el, export_item_t* parent);

    void process_symbol_instance(const element_t& el, export_item_t* parent);

    void process_symbol_item(const element_t& el, export_item_t* parent);

    void process_bitmap_instance(const element_t& el, export_item_t* parent);

    void process_bitmap_item(const element_t& el, export_item_t* library);

    void process_dynamic_text(const element_t& el, export_item_t* parent);

    void process_group(const element_t& el, export_item_t* parent);

    void process_shape(const element_t& el, export_item_t* parent);

    void render(const export_item_t& item, spritepack::atlas_t& to_atlas) const;

    [[nodiscard]]
    sg_file export_library() const;

};

}