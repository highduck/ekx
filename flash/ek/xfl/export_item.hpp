#pragma once

#include <ek/scenex/data/sg_data.hpp>

namespace ek::flash {

struct element_t;

struct export_item_t : private disable_copy_assign_t {

    sg_node_data node;

    float estimated_scale = 0.001f;
    float max_abs_scale = 100000.0f;
    const element_t* ref = nullptr;
    std::vector<export_item_t*> children;
    export_item_t* parent = nullptr;
    int usage = 0;
    int shapes = 0;

    export_item_t* drawingLayerChild = nullptr;
    element_t* drawingLayerElement = nullptr;
    int animationSpan0 = 0;
    int animationSpan1 = 0;
    bool renderThis = false;

    int fromLayer = 0;
    movie_layer_data* linkedMovieLayer = nullptr;

    export_item_t() = default;

    ~export_item_t();

    void add(export_item_t* item);

    void append_to(export_item_t* parent_);

    [[nodiscard]]
    export_item_t* find_library_item(const std::string& libraryName) const;

    void inc_ref(export_item_t& lib);

    void update_scale(export_item_t& lib, const matrix_2d& parent_matrix);

};

}


