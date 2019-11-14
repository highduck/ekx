#pragma once

#include <scenex/ek/basic_application.h>
#include <ek/editor/editor.hpp>

namespace ek {

class editor_app_t : public scenex::basic_application {
public:
    editor_app_t();

    ~editor_app_t() override;

//    void initialize() override;
//
//    void preload() override;

protected:
    ek::editor_context_t editor_;

    void preload_root_assets_pack() override;
};

}

