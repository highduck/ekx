#include "flash_doc.hpp"
#include "flash_archive.hpp"
#include "parsing/parsing.hpp"

#include <pugixml.hpp>

namespace ek::flash {

const element_t* flash_doc::find(const std::string& name, element_type type) const {
    for (const auto& s: library) {
        if (s.item.name == name && s.elementType == type) {
            return &s;
        }
    }
    return nullptr;
}

const element_t* flash_doc::find_linkage(const std::string& linkage) const {
    for (const auto& s: library) {
        if (s.item.linkageClassName == linkage) {
            return &s;
        }
    }
    return nullptr;
}

xml_document* load_xml(const basic_entry& root, const path_t& path) {
    return root.open(path)->xml();
}

void load(const basic_entry& root, flash_doc& doc) {
    auto* xml = load_xml(root, path_t{"DOMDocument.xml"});
    auto node = xml->child("DOMDocument");

    doc.info << node;

    for (const auto& item: node.child("folders").children("DOMFolderItem")) {
        doc.folders.push_back(parse_xml_node<folder_item>(item));
    }

    for (const auto& item: node.child("fonts").children("DOMFontItem")) {
        doc.library.push_back(parse_xml_node<element_t>(item));
    }

    for (const auto& item: node.child("media").children("DOMBitmapItem")) {
        auto bi = parse_xml_node<element_t>(item);
        bi.bitmap.reset(load_bitmap(*root.open(path_t{"bin"} / bi.bitmapDataHRef)));
        doc.library.push_back(std::move(bi));
    }

    for (const auto& item: node.child("media").children("DOMSoundItem")) {
        auto sound = parse_xml_node<element_t>(item);
        doc.library.push_back(std::move(sound));
    }

    for (const auto& item: node.child("symbols").children("Include")) {
        auto library_doc = load_xml(root, path_t{"LIBRARY"} / item.attribute("href").value());
        auto symbol = parse_xml_node<element_t>(library_doc->child("DOMSymbolItem"));
        doc.library.push_back(std::move(symbol));
    }

    for (const auto& item: node.child("timelines").children("DOMTimeline")) {
        element_t el;
        el.timeline = parse_xml_node<timeline_t>(item);
        if (!el.timeline.name.empty()) {
            el.elementType = element_type::scene_timeline;
            el.item.name = "_SCENE_" + el.timeline.name;
            el.item.linkageExportForAS = true;
            el.item.linkageClassName = el.item.name;
            doc.scenes[el.timeline.name] = el.item.name;
            doc.library.push_back(std::move(el));
        }
    }
}

flash_doc::flash_doc(std::unique_ptr<basic_entry> root)
        : root_{std::move(root)} {
    load(*root_, *this);
}

flash_doc::flash_doc(const path_t& path) :
        flash_doc{load_flash_archive(path)} {

}

}
