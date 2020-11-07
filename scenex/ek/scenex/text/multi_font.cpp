#include "multi_font.hpp"

namespace ek {

MultiFont::MultiFont() :
        FontImplBase(FontType::Multi) {

}

MultiFont::~MultiFont() = default;

void MultiFont::addFont(const char* name) {
    fonts.emplace_back(name);
}

void MultiFont::setBlur(float radius, int iterations, int strengthPower) {
    blurRadius_ = radius;
    blurIterations_ = iterations < 0 ? 0 : (iterations > 3 ? 3 : iterations);
    strengthPower_ = strengthPower < 0 ? 0 : (strengthPower > 7 ? 7 : strengthPower);
}

// store prerendered glyph for baseFontSize and upscaled by dpiScale
// quad scale just multiplier to get fontSize relative to baseFontSize
bool MultiFont::getGlyph(uint32_t codepoint, Glyph& outGlyph) {
    for (auto& fontAsset : fonts) {
        if (fontAsset) {
            auto* impl = fontAsset->getImpl();
            impl->setBlur(blurRadius_, blurIterations_, strengthPower_);
            if (impl->getGlyph(codepoint, outGlyph)) {
                return true;
            }
        }
    }
    return false;
}

}