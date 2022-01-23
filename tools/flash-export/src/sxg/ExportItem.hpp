#pragma once

#include <ek/format/SGFile.hpp>
#include <ek/ds/Array.hpp>
#include <memory>

namespace ek::xfl {

struct Element;

struct ExportItem {

    SGNodeData node;

    float estimated_scale = 0.001f;
    float max_abs_scale = 100000.0f;
    const Element* ref = nullptr;
    Array<ExportItem*> children;
    ExportItem* parent = nullptr;
    int usage = 0;
    int shapes = 0;

    ExportItem* drawingLayerChild = nullptr;
    std::unique_ptr<Element> drawingLayerItem = nullptr;
    std::unique_ptr<Element> drawingLayerInstance = nullptr;
    int animationSpan0 = 0;
    int animationSpan1 = 0;
    bool renderThis = false;

    int fromLayer = 0;
    bool movieLayerIsLinked = false;

    ExportItem() = default;

    ~ExportItem();

    // delete copy-move
    ExportItem(const ExportItem& v) = delete;

    ExportItem& operator=(const ExportItem& v) = delete;

    ExportItem(ExportItem&& v) = delete;

    ExportItem& operator=(ExportItem&& v) = delete;

    void add(ExportItem* item);

    void append_to(ExportItem* parent_);

    [[nodiscard]]
    ExportItem* find_library_item(const string_hash_t libraryName) const;

    void inc_ref(ExportItem& lib);

    void update_scale(ExportItem& lib, mat3x2_t parent_matrix);

};

struct processing_bag_t {
    Array<ExportItem*> list;
};

}


