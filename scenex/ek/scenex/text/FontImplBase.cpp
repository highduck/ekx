#include "FontImplBase.hpp"

namespace ek {

FontImplBase::FontImplBase(FontType fontType_) :
        fontType{fontType_},
        lineHeightMultiplier{1.f} {

}

FontImplBase::~FontImplBase() = default;

bool FontImplBase::isReady() const {
    return ready_ && loaded_;
}

}


