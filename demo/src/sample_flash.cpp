#include <ek/scenex/data/sg_factory.hpp>
#include "sample_flash.hpp"

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