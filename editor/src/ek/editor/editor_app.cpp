#include "editor_app.hpp"

namespace ek {

editor_app_t::editor_app_t()
        : editor_{*this} {

}

editor_app_t::~editor_app_t() = default;

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

}