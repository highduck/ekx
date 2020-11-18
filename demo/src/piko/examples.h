#pragma once

#include <ek/scenex/base/Script.hpp>
#include <ek/graphics/render_target.hpp>
//#include <ek/editor/gui/screen_recorder.hpp>

namespace ek::piko {

EK_DECL_SCRIPT_CPP(book) {
public:
    void draw() override;
};

EK_DECL_SCRIPT_CPP(dna) {
public:
    void draw() override;
};

EK_DECL_SCRIPT_CPP(diamonds) {
public:
    diamonds();

    void draw() override;

    graphics::render_target_t rt;
//    screen_recorder recorder;
    float time = 0.0f;
    bool first_frame = true;
};

}