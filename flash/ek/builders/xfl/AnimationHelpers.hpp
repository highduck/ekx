#pragma once

namespace ek {
struct SGKeyFrameTransform;
struct SGMovieFrameData;
}

namespace ek::xfl {

struct Frame;
struct Element;

SGKeyFrameTransform extractTweenDelta(const Frame& frame, const Element& el0, const Element& el1);

SGMovieFrameData createFrameModel(const Frame& frame);

void setupFrameFromElement(SGMovieFrameData& target, const Element& el);

}

