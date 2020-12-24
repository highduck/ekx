#pragma once

#include <ek/scenex/base/Script.hpp>
#include <ek/graphics/graphics.hpp>
#include <ek/util/signals.hpp>
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
    ~diamonds() override;
    void prerender();
    void draw() override;

    graphics::Texture* rt = nullptr;
    sg_pass pass;
//    screen_recorder recorder;
    float time = 0.0f;
    bool first_frame = true;

    signal_t<>::token ll;
};

}