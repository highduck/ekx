#pragma once

#include <ek/scenex/components/script.hpp>
#include <ek/graphics/render_target.hpp>
//#include <ek/editor/gui/screen_recorder.hpp>

namespace ek::piko {

class book : public script {
public:
    void draw() override;
};

class dna : public script {
public:
    void draw() override;
};

class diamonds : public script {
public:
    diamonds();

    void draw() override;

    render_target_t rt;
//    screen_recorder recorder;
    float time = 0.0f;
    bool first_frame = true;
};

}