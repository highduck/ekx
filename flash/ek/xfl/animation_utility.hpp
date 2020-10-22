#pragma once

#include <ek/flash/doc/flash_doc.hpp>
#include <ek/util/logger.hpp>
#include "export_item.hpp"

namespace ek::flash {

inline float sign(float a) {
    return a > 0.0f ? 1.0f : (a < 0.0f ? -1.0f : 0.0f);
}

struct keyframe_transform_t {
    float2 position;
    float2 scale{1.0f, 1.0f};
    float2 skew;
    float2 pivot;
    color_transform_f color;

    void assign(const element_t& el) {
        const auto m = el.matrix;
        position = m.transform(el.transformationPoint);
        scale = m.scale();
        skew = m.skew();
        pivot = el.transformationPoint;
        color = el.color;
    }

    keyframe_transform_t operator-(const keyframe_transform_t& v) const {
        return {
                position - v.position,
                scale - v.scale,
                skew - v.skew,
                pivot - v.pivot,
                color - v.color,
        };
    }

    keyframe_transform_t operator+(const keyframe_transform_t& v) const {
        return {
                position + v.position,
                scale + v.scale,
                skew + v.skew,
                pivot + v.pivot,
                color + v.color,
        };
    }

    void fixRotation(const keyframe_transform_t& prev) {
        if (prev.skew.x + math::pi < skew.x) {
            skew.x -= 2 * math::pi;
        } else if (prev.skew.x - math::pi > skew.x) {
            skew.x += 2 * math::pi;
        }
        if (prev.skew.y + math::pi < skew.y) {
            skew.y -= 2 * math::pi;
        } else if (prev.skew.y - math::pi > skew.y) {
            skew.y += 2 * math::pi;
        }
    }

    void addRotation(const frame_t& frame, const keyframe_transform_t& prev) {
        float additionalRotation = 0;
        const auto rotate = frame.motionTweenRotate;
        const auto times = frame.motionTweenRotateTimes;
// If a direction is specified, take it into account
        if (rotate != rotation_direction::none) {
            float direction = (rotate == rotation_direction::cw ? 1.0f : -1.0f);
// negative scales affect rotation direction
            direction *= sign(scale.x) * sign(scale.y);

            while (direction < 0 && prev.skew.x < skew.x) {
                skew.x -= 2 * math::pi;
            }
            while (direction > 0 && prev.skew.x > skew.x) {
                skew.x += 2 * math::pi;
            }
            while (direction < 0 && prev.skew.y < skew.y) {
                skew.y -= 2 * math::pi;
            }
            while (direction > 0 && prev.skew.y > skew.y) {
                skew.y += 2 * math::pi;
            }

// additional rotations specified?
            additionalRotation += 2.0 * math::pi * direction * times;
        }

        skew.x += additionalRotation;
        skew.y += additionalRotation;
    }
};


keyframe_transform_t extractTweenDelta(const frame_t& frame, const element_t& el0, const element_t& el1) {
    keyframe_transform_t t0;
    keyframe_transform_t t1;
    t0.assign(el0);
    t1.assign(el1);
    t1.fixRotation(t0);
    t1.addRotation(frame, t0);
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
    const auto m = el.matrix;
    target.pivot = el.transformationPoint;
    target.position = m.transform(el.transformationPoint);
    target.scale = m.scale();
    target.skew = m.skew();
    target.color = el.color;
    target.visible = el.isVisible;
    if (el.symbolType == symbol_type::graphic) {
        target.loopMode = static_cast<int>(el.loop);
        target.firstFrame = el.firstFrame;
    }
}

}

