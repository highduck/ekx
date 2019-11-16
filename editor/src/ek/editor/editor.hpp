#pragma once

#include <scenex/ek/basic_application.h>
#include <scenex/asset2/asset_manager.hpp>
#include <ek/editor/imgui/imgui_module.hpp>

namespace ek {

class editor_context_t {
public:
    explicit editor_context_t(scenex::basic_application& app);

    ~editor_context_t();

private:
    scenex::asset_manager_t assets;
    imgui_module_t gui_;

    signal_t<>::token t1;
    signal_t<float>::token t2;
    signal_t<>::token t3;
    scenex::basic_application* app_;
};

}