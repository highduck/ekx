#pragma once

#include <ek/scenex/ek/basic_application.hpp>
#include <ek/scenex/asset2/asset_manager.hpp>
#include <ek/editor/imgui/imgui_module.hpp>
#include <ek/editor/assets/editor_project.hpp>

namespace ek {

class editor_context_t {
public:
    explicit editor_context_t(basic_application& app);

    ~editor_context_t();

    editor_project_t project;

private:
    imgui_module_t gui_;

    signal_t<>::token t1;
    signal_t<float>::token t2;
    signal_t<>::token t3;
    basic_application* app_;
};

}