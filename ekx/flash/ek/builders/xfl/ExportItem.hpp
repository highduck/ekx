#pragma once

#include <ek/scenex/data/SGFile.hpp>

namespace ek::xfl {

struct Element;

struct ExportItem : private disable_copy_assign_t {

    SGNodeData node;

    float estimated_scale = 0.001f;
    float max_abs_scale = 100000.0f;
    const Element* ref = nullptr;
    std::vector<ExportItem*> children;
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

    void add(ExportItem* item);

    void append_to(ExportItem* parent_);

    [[nodiscard]]
    ExportItem* find_library_item(const std::string& libraryName) const;

    void inc_ref(ExportItem& lib);

    void update_scale(ExportItem& lib, const matrix_2d& parent_matrix);

};

struct processing_bag_t {
    std::vector<ExportItem*> list;
};

}


