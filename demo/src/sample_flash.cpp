#include "sample_flash.hpp"

#include <ek/scenex/data/SGFile.hpp>
#include <ek/scenex/base/Node.hpp>

namespace ek {

SampleFlash::SampleFlash(const char* scene) :
        SampleBase() {
    title = scene;
    for (auto& c: title) {
        c = toupper(c);
    }

    append(container, sg_create("tests", scene));
}
}