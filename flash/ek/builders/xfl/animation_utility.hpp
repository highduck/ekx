#pragma once

#include <ek/xfl/Doc.hpp>
#include <ek/util/logger.hpp>
#include "export_item.hpp"

namespace ek::xfl {

inline float sign(float a) {
    return a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f);
}

keyframe_transform_t create_transform(const element_t& el) {
    const auto m = el.matrix;
    keyframe_transform_t r;
    r.position = m.transform(el.transformationPoint);
    r.scale = m.scale();
    r.skew = m.skew();
    r.pivot = el.transformationPoint;
    r.color = el.color;
    return r;
}

void fixRotation(keyframe_transform_t& curr, const keyframe_transform_t& prev) {
    if (prev.skew.x + math::pi < curr.skew.x) {
        curr.skew.x -= 2 * math::pi;
    } else if (prev.skew.x - math::pi > curr.skew.x) {
        curr.skew.x += 2 * math::pi;
    }
    if (prev.skew.y + math::pi < curr.skew.y) {
        curr.skew.y -= 2 * math::pi;
    } else if (prev.skew.y - math::pi > curr.skew.y) {
        curr.skew.y += 2 * math::pi;
    }
}

void addRotation(keyframe_transform_t& curr, const frame_t& frame, const keyframe_transform_t& prev) {
    double additionalRotation = 0;
    const auto rotate = frame.motionTweenRotate;
    const auto times = frame.motionTweenRotateTimes;
// If a direction is specified, take it into account
    if (rotate != rotation_direction::none) {
        float direction = (rotate == rotation_direction::cw ? 1.0f : -1.0f);
// negative scales affect rotation direction
        direction *= sign(curr.scale.x) * sign(curr.scale.y);

        while (direction < 0 && prev.skew.x < curr.skew.x) {
            curr.skew.x -= 2 * math::pi;
        }
        while (direction > 0 && prev.skew.x > curr.skew.x) {
            curr.skew.x += 2 * math::pi;
        }
        while (direction < 0 && prev.skew.y < curr.skew.y) {
            curr.skew.y -= 2 * math::pi;
        }
        while (direction > 0 && prev.skew.y > curr.skew.y) {
            curr.skew.y += 2 * math::pi;
        }

// additional rotations specified?
        additionalRotation += 2.0 * math::pi * direction * times;
    }

    curr.skew.x += additionalRotation;
    curr.skew.y += additionalRotation;
}

keyframe_transform_t extractTweenDelta(const frame_t& frame, const element_t& el0, const element_t& el1) {
    auto t0 = create_transform(el0);
    auto t1 = create_transform(el1);
    fixRotation(t1, t0);
    addRotation(t1, frame, t0);
    return t1 - t0;
}

movie_frame_data createFrameModel(const frame_t& frame) {
    movie_frame_data ef;
    ef.index = frame.index;
    ef.duration = frame.duration;
    if (frame.tweenType == tween_type::classic) {
        ef.motion_type = 1;
        for (const auto& fd : frame.tweens) {
            auto& g = ef.easing.emplace_back();
            g.attribute = static_cast<uint8_t>(fd.target);
            g.ease = static_cast<float>(fd.intensity) / 100.0f;
            g.curve = fd.custom_ease;
        }
        if (ef.easing.empty()) {
            auto& g = ef.easing.emplace_back();
            g.attribute = 0;
            g.ease = static_cast<float>(frame.acceleration) / 100.0f;
        }
        ef.rotate = (int) frame.motionTweenRotate;
        ef.rotateTimes = frame.motionTweenRotateTimes;
    } else if (frame.tweenType == tween_type::motion_object) {
        EK_WARN << "motion object is not supported";
    }
    return ef;
}

void setupFrameFromElement(movie_frame_data& target, const element_t& el) {
    target.transform = create_transform(el);
    target.visible = el.isVisible;
    if (el.symbolType == symbol_type::graphic) {
        target.loopMode = static_cast<int>(el.loop);
        target.firstFrame = el.firstFrame;
    }
}

}

