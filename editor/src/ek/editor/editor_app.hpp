#pragma once

#include <ek/scenex/app/basic_application.hpp>
#include <ek/editor/editor.hpp>

namespace ek {

class editor_app_t : public basic_application {
public:
    editor_app_t();

    ~editor_app_t() override;

    void initialize() override;
//
//    void preload() override;

protected:
    std::unique_ptr<editor_context_t> editor_;

    void preload_root_assets_pack() override;
};

}

