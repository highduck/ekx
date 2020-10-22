#include "editor_app.hpp"

#include <memory>

namespace ek {

editor_app_t::editor_app_t()
        : basic_application() {

}

editor_app_t::~editor_app_t() {
    basic_application::~basic_application();
};

//void editor_app_t::initialize() {
//    basic_application::initialize();
//}
//
//void editor_app_t::preload() {
//    basic_application::preload();
//}

void editor_app_t::preload_root_assets_pack() {
    // skip
}

void editor_app_t::initialize() {
    basic_application::initialize();
    editor_ = std::make_unique<editor_context_t>(*this);
}

}