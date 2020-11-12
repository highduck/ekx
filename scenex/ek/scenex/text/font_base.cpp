#include "font_base.hpp"

namespace ek {

FontImplBase::FontImplBase(FontType fontType_) :
        fontType{fontType_},
        lineHeightMultiplier{1.f} {

}

FontImplBase::~FontImplBase() = default;

}
